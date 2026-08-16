#include "InputSystem.h"
#include <cmath>

InputSystem::InputSystem()
{
}

void InputSystem::update()
{
    tankX = 0.0f;
    tankY = 0.0f;

    // Приналичии геймпада отключаем ввод клавиатурой/мышью
    if (IsGamepadAvailable(0))
    {
        m_mouseEnabled = false;
        m_mousePos = {0.0f, 0.0f};
        m_mouseLeftPressed = false;

        // Сохраняем предыдущее состояние
        for (int i = 0; i < MAX_GAMEPAD_BUTTONS; i++)
            m_gamepadPrevDown[i] = m_gamepadDown[i];

        // Читаем текущее состояние всех кнопок геймпада
        updateGamepadState();

        // Тач
        int touchCount = GetTouchPointCount();
        m_touchPressed = (touchCount > 0 && m_prevTouchCount == 0);
        if (touchCount > 0)
            m_touchPos = GetTouchPosition(0);
        m_prevTouchCount = touchCount;

        // Левый стик
        float moveX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float moveY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

        if (fabsf(moveX) > DEADZONE)
            tankX -= (moveX > 0.0f) ? 1.0f : -1.0f;
        if (fabsf(moveY) > DEADZONE)
            tankY -= (moveY > 0.0f) ? 1.0f : -1.0f;

        // Определение направления левого стика
        m_prevLStickDir = m_currLStickDir;
        if (fabsf(moveX) > DEADZONE || fabsf(moveY) > DEADZONE)
        {
            // Определяем доминирующую ось
            if (fabsf(moveX) > fabsf(moveY))
            {
                m_currLStickDir = (moveX > 0.0f) ? StickDirection::Right : StickDirection::Left;
            }
            else
            {
                m_currLStickDir = (moveY > 0.0f) ? StickDirection::Down : StickDirection::Up;
            }
        }
        else
        {
            m_currLStickDir = StickDirection::None;
        }

        // D-pad
        if (isGamepadButtonDown(GAMEPAD_BUTTON_LEFT_FACE_LEFT))
            tankX += 1.0f;
        if (isGamepadButtonDown(GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
            tankX -= 1.0f;
        if (isGamepadButtonDown(GAMEPAD_BUTTON_LEFT_FACE_UP))
            tankY -= 1.0f;
        if (isGamepadButtonDown(GAMEPAD_BUTTON_LEFT_FACE_DOWN))
            tankY += 1.0f;
    }
    else
    {
        m_mouseEnabled = true;
        m_mousePos = GetMousePosition();
        m_mouseLeftPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        if (IsKeyDown(KEY_A))
            tankX += 1.0f;
        if (IsKeyDown(KEY_D))
            tankX -= 1.0f;
        if (IsKeyDown(KEY_W))
            tankY -= 1.0f;
        if (IsKeyDown(KEY_S))
            tankY += 1.0f;

        if (IsKeyDown(KEY_LEFT))
            tankX += 1.0f;
        if (IsKeyDown(KEY_RIGHT))
            tankX -= 1.0f;
        if (IsKeyDown(KEY_UP))
            tankY -= 1.0f;
        if (IsKeyDown(KEY_DOWN))
            tankY += 1.0f;
    }

    // Клампим значения
    if (tankX > 1.0f)
        tankX = 1.0f;
    if (tankX < -1.0f)
        tankX = -1.0f;
    if (tankY > 1.0f)
        tankY = 1.0f;
    if (tankY < -1.0f)
        tankY = -1.0f;
}

float InputSystem::getTankX() const { return tankX; }
float InputSystem::getTankY() const { return tankY; }

bool InputSystem::isTankMoved() const
{
    return fabsf(tankX) > 0.0f || fabsf(tankY) > 0.0f;
}

bool InputSystem::isFirePressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyDown(KEY_SPACE);
        c = c || IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    }
    else
    {
        c = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT); // A
    }
    return c;
}

bool InputSystem::isRearViewPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyDown(KEY_RIGHT_ALT);
        c = c || IsKeyDown(KEY_LEFT_ALT);
    }
    else
    {
        c = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_UP); // X
    }
    return c;
}

bool InputSystem::isTurboPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_RIGHT_CONTROL);
        c = c || IsKeyPressed(KEY_LEFT_CONTROL);
    }
    else
    {
        c = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN); // B
    }
    return c;
}

bool InputSystem::isToggleIdPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_T);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_RIGHT_FACE_LEFT); // Y
    }
    return c;
}

bool InputSystem::isNextTankPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_E);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_RIGHT_TRIGGER_1); // R1
    }
    return c;
}

bool InputSystem::isPrevTankPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_Q);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_LEFT_TRIGGER_1); // L1
    }
    return c;
}

bool InputSystem::isQuitPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_ESCAPE);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_MIDDLE_LEFT); // Select
    }
    return c;
}

void InputSystem::setTankSelected(int t)
{
    if (t >= PLAYER_MIN && t <= PLAYER_MAX)
        tankSelected = t;
    else
        tankSelected = PLAYER_MIN;
}

int InputSystem::getRequestedTank()
{
    if (m_mouseEnabled)
    {
        for (int i = 0; i < 12; i++)
        {
            if (IsKeyPressed(KEY_F1 + i))
            {
                tankSelected = i + 1;
                return tankSelected;
            }
        }
    }
    else
    {
        if (isNextTankPressed())
        {
            tankSelected++;
            if (tankSelected > PLAYER_MAX)
                tankSelected = PLAYER_MAX;
            return tankSelected;
        }
        if (isPrevTankPressed())
        {
            tankSelected--;
            if (tankSelected < PLAYER_MIN)
                tankSelected = PLAYER_MIN;
            return tankSelected;
        }
    }

    return 0;
}

// === Меню ===
bool InputSystem::isMenuLeftPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_LEFT);
        c = c || IsKeyPressed(KEY_A);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_LEFT_TRIGGER_2);                            // L2
        c = c || isGamepadButtonJustPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT);                       // D-pad left
        c = c || (m_currLStickDir == StickDirection::Left && m_prevLStickDir != m_currLStickDir); // LS left
    }
    return c;
}

bool InputSystem::isMenuRightPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_RIGHT);
        c = c || IsKeyPressed(KEY_D);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_RIGHT_TRIGGER_2);                            // R2
        c = c || isGamepadButtonJustPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT);                       // D-pad right
        c = c || (m_currLStickDir == StickDirection::Right && m_prevLStickDir != m_currLStickDir); // LS right
    }
    return c;
}

bool InputSystem::isMenuUpPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_UP);
        c = c || IsKeyPressed(KEY_W);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_LEFT_TRIGGER_1);                          // L1
        c = c || isGamepadButtonJustPressed(GAMEPAD_BUTTON_LEFT_FACE_UP);                       // D-pad up
        c = c || (m_currLStickDir == StickDirection::Up && m_prevLStickDir != m_currLStickDir); // LS up
    }
    return c;
}

bool InputSystem::isMenuDownPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_DOWN);
        c = c || IsKeyPressed(KEY_S);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_RIGHT_TRIGGER_1);                           // R1
        c = c || isGamepadButtonJustPressed(GAMEPAD_BUTTON_LEFT_FACE_DOWN);                       // D-pad down
        c = c || (m_currLStickDir == StickDirection::Down && m_prevLStickDir != m_currLStickDir); // LS down
    }
    return c;
}

bool InputSystem::isMenuConfirmPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_ENTER);
        c = c || IsKeyPressed(KEY_SPACE);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT); // A
    }
    return c;
}

bool InputSystem::isMenuCancelPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_BACKSPACE);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN); // B
    }
    return c;
}

bool InputSystem::isMenuSpecial1Pressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_X);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_RIGHT_FACE_UP); // X
    }
    return c;
}

bool InputSystem::isMenuSpecial2Pressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_Y);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_RIGHT_FACE_LEFT); // Y
    }
    return c;
}

bool InputSystem::isMenuBackPressed() const
{
    return isQuitPressed();
}

bool InputSystem::isMenuNextPressed() const
{
    bool c = false;
    if (m_mouseEnabled)
    {
        c = IsKeyPressed(KEY_ENTER);
    }
    else
    {
        c = isGamepadButtonJustPressed(GAMEPAD_BUTTON_MIDDLE_RIGHT); // Start
    }
    return c;
}

// === Геймпад ===
bool InputSystem::isGamepadConnected() const
{
    return IsGamepadAvailable(0);
}

bool InputSystem::isGamepadAvailable() const
{
    return IsGamepadAvailable(0);
}

void InputSystem::updateGamepadState()
{
    for (int i = 0; i < MAX_GAMEPAD_BUTTONS; i++)
    {
        m_gamepadDown[i] = IsGamepadButtonDown(0, i);
    }
}

bool InputSystem::isGamepadButtonDown(int button) const
{
    if (button < 0 || button >= MAX_GAMEPAD_BUTTONS)
        return false;
    return m_gamepadDown[button];
}

bool InputSystem::isGamepadButtonJustPressed(int button) const
{
    if (button < 0 || button >= MAX_GAMEPAD_BUTTONS)
        return false;
    return m_gamepadDown[button] && !m_gamepadPrevDown[button];
}

bool InputSystem::isGamepadAnyPressed(std::initializer_list<int> buttons) const
{
    for (int btn : buttons)
    {
        if (isGamepadButtonJustPressed(btn))
            return true;
    }
    return false;
}