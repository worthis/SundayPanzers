#pragma once
#include "raylib.h"
#include "GameConfig.h"
#include <initializer_list>

class InputSystem
{
public:
    InputSystem();

    void update();

    // Управление танком (для боя)
    float getTankX() const;
    float getTankY() const;
    bool isTankMoved() const;
    bool isFirePressed() const;     // MBL, Space / A
    bool isTurboPressed() const;    // Control / B
    bool isRearViewPressed() const; // Alt / X
    int getRequestedTank();

    // === Меню ===
    bool isMenuLeftPressed() const;     // Стрелка влево, LT, DPad влево, LS влево
    bool isMenuRightPressed() const;    // Стрелка вправо, RT, DPad вправо, LS вправо
    bool isMenuUpPressed() const;       // Стрелка вверх, L1, DPad вверх, LS вверх
    bool isMenuDownPressed() const;     // Стрелка вниз, R1, DPad вниз, LS вниз
    bool isMenuConfirmPressed() const;  // Space, Enter, A
    bool isMenuCancelPressed() const;   // Esc, B
    bool isMenuSpecial1Pressed() const; // X, X
    bool isMenuSpecial2Pressed() const; // Y, Y
    bool isMenuBackPressed() const;     // Esc / Select
    bool isMenuNextPressed() const;     // Enter / Start

    // === Действия ===
    bool isToggleIdPressed() const; // T / Y
    bool isNextTankPressed() const; // E / R1
    bool isPrevTankPressed() const; // Q / L1
    void setTankSelected(int t);
    bool isQuitPressed() const; // Esc / Select

    // === Мышь ===
    bool isMouseEnabled() const { return m_mouseEnabled; }
    Vector2 getMousePosition() const { return m_mousePos; }
    bool isMouseLeftPressed() const { return m_mouseLeftPressed; }

    // === Геймпад ===
    bool isGamepadAvailable() const;
    bool isGamepadConnected() const;
    bool isGamepadButtonDown(int button) const;
    bool isGamepadButtonJustPressed(int button) const;
    bool isGamepadAnyPressed(std::initializer_list<int> buttons) const;

    // === Тач ===
    bool isTouchPressed() const { return m_touchPressed; }
    Vector2 getTouchPosition() const { return m_touchPos; }

private:
    static constexpr float DEADZONE = 0.15f;
    static constexpr int MAX_GAMEPAD_BUTTONS = 32;

    float tankX = 0.0f, tankY = 0.0f;
    int tankSelected = 0;

    bool m_mouseEnabled = false;
    Vector2 m_mousePos = {0, 0};
    bool m_mouseLeftPressed = false;

    Vector2 m_touchPos = {0.0f, 0.0f};
    bool m_touchPressed = false;
    int m_prevTouchCount = 0;

    enum class StickDirection
    {
        None,
        Left,
        Right,
        Up,
        Down
    };
    StickDirection m_prevLStickDir = StickDirection::None;
    StickDirection m_currLStickDir = StickDirection::None;

    bool m_gamepadDown[MAX_GAMEPAD_BUTTONS] = {};     // текущее состояние
    bool m_gamepadPrevDown[MAX_GAMEPAD_BUTTONS] = {}; // предыдущий кадр

    void updateGamepadState();
};