#include "InputSystem.h"
#include "InputMappings.h"
#include <cmath>

// Вспомогательные функции для проверки нажатия клавиш
static bool isAnyKeyDown(const std::vector<int> &keys)
{
    for (int key : keys)
    {
        if (IsKeyDown(key))
            return true;
    }

    return false;
}

static bool isAnyKeyPressed(const std::vector<int> &keys)
{
    for (int key : keys)
    {
        if (IsKeyPressed(key))
            return true;
    }

    return false;
}

static bool isAnyMouseButtonDown(const std::vector<int> &buttons)
{
    for (int btn : buttons)
    {
        if (IsMouseButtonDown(btn))
            return true;
    }

    return false;
}

static bool isAnyMouseButtonPressed(const std::vector<int> &buttons)
{
    for (int btn : buttons)
    {
        if (IsMouseButtonPressed(btn))
            return true;
    }

    return false;
}

static bool isAnyGamepadButtonDown(const std::vector<int> &buttons)
{
    if (!IsGamepadAvailable(0))
        return false;

    for (int btn : buttons)
    {
        if (IsGamepadButtonDown(0, btn))
            return true;
    }

    return false;
}

static bool isAnyGamepadButtonPressed(const std::vector<int> &buttons)
{
    if (!IsGamepadAvailable(0))
        return false;

    for (int btn : buttons)
    {
        if (IsGamepadButtonPressed(0, btn))
            return true;
    }

    return false;
}

InputSystem::InputSystem()
{
}

void InputSystem::update()
{
    tankX = 0.0f;
    tankY = 0.0f;

    const InputConfig &keys = config.getInputConfig();

    // Приналичии геймпада отключаем ввод клавиатурой/мышью
    if (IsGamepadAvailable(0))
    {
        m_mouseEnabled = false;
        m_mousePos = {0.0f, 0.0f};
        m_mouseLeftPressed = false;

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
        if (isAnyGamepadButtonDown(keys.tankLeft.gamepad))
            tankX += 1.0f;
        if (isAnyGamepadButtonDown(keys.tankRight.gamepad))
            tankX -= 1.0f;
        if (isAnyGamepadButtonDown(keys.tankForward.gamepad))
            tankY -= 1.0f;
        if (isAnyGamepadButtonDown(keys.tankBackward.gamepad))
            tankY += 1.0f;
    }
    else
    {
        m_mouseEnabled = true;
        m_mousePos = GetMousePosition();
        m_mouseLeftPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        if (isAnyKeyDown(keys.tankLeft.keyboard))
            tankX += 1.0f;
        if (isAnyKeyDown(keys.tankRight.keyboard))
            tankX -= 1.0f;
        if (isAnyKeyDown(keys.tankForward.keyboard))
            tankY -= 1.0f;
        if (isAnyKeyDown(keys.tankBackward.keyboard))
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
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyDown(keys.fire.keyboard) ||
               isAnyMouseButtonDown(keys.fire.mouse);
    }
    else
    {
        return isAnyGamepadButtonDown(keys.fire.gamepad);
    }
}

bool InputSystem::isRearViewPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyDown(keys.rearView.keyboard) ||
               isAnyMouseButtonDown(keys.rearView.mouse);
    }
    else
    {
        return isAnyGamepadButtonDown(keys.rearView.gamepad);
    }
}

bool InputSystem::isTurboPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.turbo.keyboard) ||
               isAnyMouseButtonPressed(keys.turbo.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.turbo.gamepad);
    }
}

bool InputSystem::isToggleIdPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.toggleId.keyboard) ||
               isAnyMouseButtonPressed(keys.toggleId.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.toggleId.gamepad);
    }
}

bool InputSystem::isNextTankPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.nextTank.keyboard) ||
               isAnyMouseButtonPressed(keys.nextTank.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.nextTank.gamepad);
    }
}

bool InputSystem::isPrevTankPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.prevTank.keyboard) ||
               isAnyMouseButtonPressed(keys.prevTank.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.prevTank.gamepad);
    }
}

bool InputSystem::isQuitPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.quit.keyboard) ||
               isAnyMouseButtonPressed(keys.quit.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.quit.gamepad);
    }
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

    return 0;
}

// === Меню ===
bool InputSystem::isMenuLeftPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.menuLeft.keyboard) ||
               isAnyMouseButtonPressed(keys.menuLeft.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.menuLeft.gamepad) ||
               (m_currLStickDir == StickDirection::Left && m_prevLStickDir != m_currLStickDir); // LS left
    }
}

bool InputSystem::isMenuRightPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.menuRight.keyboard) ||
               isAnyMouseButtonPressed(keys.menuRight.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.menuRight.gamepad) ||
               (m_currLStickDir == StickDirection::Right && m_prevLStickDir != m_currLStickDir); // LS right
    }
}

bool InputSystem::isMenuUpPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.menuUp.keyboard) ||
               isAnyMouseButtonPressed(keys.menuUp.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.menuUp.gamepad) ||
               (m_currLStickDir == StickDirection::Up && m_prevLStickDir != m_currLStickDir); // LS up
    }
}

bool InputSystem::isMenuDownPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.menuDown.keyboard) ||
               isAnyMouseButtonPressed(keys.menuDown.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.menuDown.gamepad) ||
               (m_currLStickDir == StickDirection::Down && m_prevLStickDir != m_currLStickDir); // LS down
    }
}

bool InputSystem::isMenuConfirmPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.menuConfirm.keyboard) ||
               isAnyMouseButtonPressed(keys.menuConfirm.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.menuConfirm.gamepad);
    }
}

bool InputSystem::isMenuCancelPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.menuCancel.keyboard) ||
               isAnyMouseButtonPressed(keys.menuCancel.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.menuCancel.gamepad);
    }
}

bool InputSystem::isMenuSpecial1Pressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.menuSpecial1.keyboard) ||
               isAnyMouseButtonPressed(keys.menuSpecial1.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.menuSpecial1.gamepad);
    }
}

bool InputSystem::isMenuSpecial2Pressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.menuSpecial2.keyboard) ||
               isAnyMouseButtonPressed(keys.menuSpecial2.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.menuSpecial2.gamepad);
    }
}

bool InputSystem::isMenuBackPressed() const
{
    return isQuitPressed();
}

bool InputSystem::isMenuNextPressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.menuNext.keyboard) ||
               isAnyMouseButtonPressed(keys.menuNext.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.menuNext.gamepad);
    }
}

bool InputSystem::isMenuFirePressed() const
{
    const InputConfig &keys = config.getInputConfig();

    if (m_mouseEnabled)
    {
        return isAnyKeyPressed(keys.menuFire.keyboard) ||
               isAnyMouseButtonPressed(keys.menuFire.mouse);
    }
    else
    {
        return isAnyGamepadButtonPressed(keys.menuFire.gamepad);
    }
}