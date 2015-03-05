
#ifndef GAINPUTINPUTDEVICEMOUSEWIN_H_
#define GAINPUTINPUTDEVICEMOUSEWIN_H_

#include "GainputInputDeviceMouseImpl.h"
#include "../GainputHelpers.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <Windowsx.h>

namespace gainput
{

class InputDeviceMouseImplWin : public InputDeviceMouseImpl
{
public:
	InputDeviceMouseImplWin(InputManager& manager, DeviceId device, InputState& state, InputState& previousState) :
		manager_(manager),
		device_(device),
		state_(&state),
		previousState_(&previousState),
		nextState_(manager.GetAllocator(), MouseButtonCount + MouseAxisCount),
		delta_(0)
	{
	}

	InputDevice::DeviceVariant GetVariant() const
	{
		return InputDevice::DV_STANDARD;
	}

	void Update(InputDeltaState* delta)
	{
		delta_ = delta;

		// Reset mouse wheel buttons
		nextState_.Set(MouseButton3, false);
		nextState_.Set(MouseButton4, false);

		if (delta)
		{
			bool oldValue = previousState_->GetBool(MouseButton3);
			if (oldValue)
			{
				delta->AddChange(device_, MouseButton3, oldValue, false);
			}
			oldValue = previousState_->GetBool(MouseButton4);
			if (oldValue)
			{
				delta->AddChange(device_, MouseButton4, oldValue, false);
			}
		}

		*state_ = nextState_;
	}

	void HandleMessage(const MSG& msg)
	{
		GAINPUT_ASSERT(state_);
		GAINPUT_ASSERT(previousState_);

		DeviceButtonId buttonId;
		bool pressed;
		bool moveMessage = false;
		int ax = -1;
		int ay = -1;
		switch (msg.message)
		{
		case WM_LBUTTONDOWN:
			buttonId = MouseButtonLeft;
			pressed = true;
			break;
		case WM_LBUTTONUP:
			buttonId = MouseButtonLeft;
			pressed = false;
			break;
		case WM_RBUTTONDOWN:
			buttonId = MouseButtonRight;
			pressed = true;
			break;
		case WM_RBUTTONUP:
			buttonId = MouseButtonRight;
			pressed = false;
			break;
		case WM_MBUTTONDOWN:
			buttonId = MouseButtonMiddle;
			pressed = true;
			break;
		case WM_MBUTTONUP:
			buttonId = MouseButtonMiddle;
			pressed = false;
			break;
		case WM_XBUTTONDOWN:
			buttonId = MouseButton4 + GET_XBUTTON_WPARAM(msg.wParam);
			pressed = true;
			break;
		case WM_XBUTTONUP:
			buttonId = MouseButton4 + GET_XBUTTON_WPARAM(msg.wParam);
			pressed = false;
			break;
		case WM_MOUSEMOVE:
			moveMessage = true;
			ax = GET_X_LPARAM(msg.lParam);
			ay = GET_Y_LPARAM(msg.lParam);
			break;
		case WM_MOUSEWHEEL:
			{
				int wheel = GET_WHEEL_DELTA_WPARAM(msg.wParam);
				if (wheel < 0)
				{
					buttonId = MouseButton4;
					pressed = true;
				}
				else if (wheel > 0)
				{
					buttonId = MouseButton3;
					pressed = true;
				}
				break;
			}
		default: // Non-mouse message
			return;
		}

		if (moveMessage)
		{
			float x = float(ax)/float(manager_.GetDisplayWidth());
			float y = float(ay)/float(manager_.GetDisplayHeight());

			if (delta_)
			{
				const float oldX = nextState_.GetFloat(MouseAxisX);
				const float oldY = nextState_.GetFloat(MouseAxisY);
				if (oldX != x)
				{
					delta_->AddChange(device_, MouseAxisX, oldX, x);
				}
				if (oldY != y)
				{
					delta_->AddChange(device_, MouseAxisY, oldY, y);
				}
			}

			nextState_.Set(MouseAxisX, x);
			nextState_.Set(MouseAxisY, y);

#ifdef GAINPUT_DEBUG
			GAINPUT_LOG("Mouse: %f, %f\n", x, y);
#endif
		}
		else
		{
			if (delta_)
			{
				const bool oldValue = nextState_.GetBool(buttonId);
				if (oldValue != pressed)
				{
					delta_->AddChange(device_, buttonId, oldValue, pressed);
				}
			}

			nextState_.Set(buttonId, pressed);

#ifdef GAINPUT_DEBUG
			GAINPUT_LOG("Button: %i: %i\n", buttonId, pressed);
#endif
		}
	}

private:
	InputManager& manager_;
	DeviceId device_;
	InputState* state_;
	InputState* previousState_;
	InputState nextState_;
	InputDeltaState* delta_;
};

}

#endif

