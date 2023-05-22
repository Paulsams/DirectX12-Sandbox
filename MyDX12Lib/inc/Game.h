/**
 *   @brief The Game class is the abstract base class for DirecX 12 demos.
 */
#pragma once

#include <Events.h>

#include <memory>
#include <string>

class Window;

class Game : public std::enable_shared_from_this<Game>
{
public:
    /**
     * Create the DirectX demo using the specified window dimensions.
     */
    Game(const std::wstring& name, int width, int height, bool vSync);
    virtual ~Game();

    int GetClientWidth() const
    {
        return m_Width;
    }

    int GetClientHeight() const
    {
        return m_Height;
    }

    /**
     *  Initialize the DirectX Runtime.
     */
    virtual bool Initialize();

    /**
     *  Load content required for the demo.
     */
    virtual bool LoadContent() = 0;

    /**
     *  Unload demo specific content that was loaded in LoadContent.
     */
    virtual void UnloadContent() = 0;

    /**
     * Destroy any resource that are used by the game.
     */
    virtual void Destroy();

protected:
    friend class Window;
    
    /**
     *  Update the game logic.
     */
    virtual void OnUpdate(UpdateEventArgs& args);

    /**
     *  Render stuff.
     */
    virtual void OnRender(RenderEventArgs& args);

    /**
     * Invoked by the registered window when a key is pressed
     * while the window has focus.
     */
    virtual void OnKeyPressed(KeyEventArgs& args);

    /**
     * Invoked when a key on the keyboard is released.
     */
    virtual void OnKeyReleased(KeyEventArgs& args);

    /**
     * Invoked when the mouse is moved over the registered window.
     */
    virtual void OnMouseMoved(MouseMotionEventArgs& args);

    /**
     * Invoked when a mouse button is pressed over the registered window.
     */
    virtual void OnMouseButtonPressed(MouseButtonEventArgs& args);

    /**
     * Invoked when a mouse button is released over the registered window.
     */
    virtual void OnMouseButtonReleased(MouseButtonEventArgs& args);

    /**
     * Invoked when the mouse wheel is scrolled while the registered window has focus.
     */
    virtual void OnMouseWheel(MouseWheelEventArgs& args);

    /**
     * Invoked when the attached window is resized.
     */
    virtual void OnResize(ResizeEventArgs& args);

    /**
     * Invoked when the registered window instance is destroyed.
     */
    virtual void OnWindowDestroy();

    std::shared_ptr<Window> m_pWindow;

private:
    std::wstring m_Name;
    int m_Width;
    int m_Height;
    bool m_vSync;
    double m_deltaTime;
    double m_totalTime;

};