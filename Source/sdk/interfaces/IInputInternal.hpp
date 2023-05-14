#pragma once

#include "IInputSystem.hpp"

typedef int HInputContext;

class IInputInternal
{
public:
	// processes input for a frame
	virtual void RunFrame() = 0;

	virtual void UpdateMouseFocus(int x, int y) = 0;

	// called when a panel becomes invalid
	virtual void PanelDeleted(unsigned int panel) = 0;

	// inputs into vgui input handling 
	virtual bool InternalCursorMoved(int x, int y) = 0; //expects input in surface space
	virtual bool InternalMousePressed(ButtonCode_t code) = 0;
	virtual bool InternalMouseDoublePressed(ButtonCode_t code) = 0;
	virtual bool InternalMouseReleased(ButtonCode_t code) = 0;
	virtual bool InternalMouseWheeled(int delta) = 0;
	virtual bool InternalKeyCodePressed(ButtonCode_t code) = 0;
	virtual void InternalKeyCodeTyped(ButtonCode_t code) = 0;
	virtual void InternalKeyTyped(wchar_t unichar) = 0;
	virtual bool InternalKeyCodeReleased(ButtonCode_t code) = 0;

	//=============================================================================
	// HPE_BEGIN
	// [dwenger] Handle gamepad joystick movement.
	//=============================================================================
	virtual bool InternalJoystickMoved(int axis, int value) = 0;
	//=============================================================================
	// HPE_END
	//=============================================================================

	// Creates/ destroys "input" contexts, which contains information
	// about which controls have mouse + key focus, for example.
	virtual HInputContext CreateInputContext() = 0;
	virtual void DestroyInputContext(HInputContext context) = 0;

	// Associates a particular panel with an input context
	// Associating NULL is valid; it disconnects the panel from the context
	virtual void AssociatePanelWithInputContext(HInputContext context, unsigned int pRoot) = 0;

	// Activates a particular input context, use DEFAULT_INPUT_CONTEXT
	// to get the one normally used by VGUI
	virtual void ActivateInputContext(HInputContext context) = 0;

	// This method is called to post a cursor message to the current input context
	virtual void PostCursorMessage() = 0;

	// Cursor position; this is the current position read from the input queue.
	// We need to set it because client code may read this during Mouse Pressed
	// events, etc.
	virtual void UpdateCursorPosInternal(int x, int y) = 0;

	//=============================================================================
	// HPE_BEGIN
	// [dwenger] Handle gamepad joystick movement.
	//=============================================================================
	virtual void UpdateJoystickXPosInternal(int pos) = 0;
	virtual void UpdateJoystickYPosInternal(int pos) = 0;

	virtual int GetJoystickXPos() = 0;
	virtual int GetJoystickYPos() = 0;
	//=============================================================================
	// HPE_END
	//=============================================================================

	// Called to handle explicit calls to CursorSetPos after input processing is complete
	virtual void HandleExplicitSetCursor() = 0;

	// Resets a particular input context, use DEFAULT_INPUT_CONTEXT
	// to get the one normally used by VGUI
	virtual void ResetInputContext(HInputContext context) = 0;

	// Updates the internal key/mouse state associated with the current input context without sending messages
	virtual void SetKeyCodeState(ButtonCode_t code, bool bPressed) = 0;
	virtual void SetMouseCodeState(ButtonCode_t code, MouseCodeState_t state) = 0;
	virtual void UpdateButtonState(const InputEvent_t& event) = 0;
};