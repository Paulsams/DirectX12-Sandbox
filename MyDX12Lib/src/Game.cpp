#include <DX12LibPCH.h>

#include <Application.h>
#include <Game.h>
#include <Window.h>

Game::Game( const std::wstring& name, int width, int height, bool vSync )
    : m_Name( name )
    , m_Width( width )
    , m_Height( height )
    , m_vSync( vSync )
{
}

Game::~Game()
{
    assert(!m_pWindow && "Use Game::Destroy() before destruction.");
}

bool Game::Initialize()
{
    // Check for DirectX Math library support.
    if (!DirectX::XMVerifyCPUSupport())
    {
        MessageBoxA(nullptr, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    m_pWindow = Application::Get().CreateRenderWindow(m_Name, m_Width, m_Height, m_vSync);
    m_pWindow->RegisterCallbacks(shared_from_this());
    m_pWindow->Show();

    return true;
}

void Game::Destroy()
{
    Application::Get().DestroyWindow(m_pWindow);
    m_pWindow.reset();
}

void Game::OnUpdate(UpdateEventArgs& args)
{
    
}

void Game::OnRender(RenderEventArgs& args)
{

}

void Game::OnKeyPressed(KeyEventArgs& args)
{
    // By default, do nothing.
}

void Game::OnKeyReleased(KeyEventArgs& args)
{
    // By default, do nothing.
}

void Game::OnMouseMoved(class MouseMotionEventArgs& args)
{
    // By default, do nothing.
}

void Game::OnMouseButtonPressed(MouseButtonEventArgs& args)
{
    // By default, do nothing.
}

void Game::OnMouseButtonReleased(MouseButtonEventArgs& args)
{
    // By default, do nothing.
}

void Game::OnMouseWheel(MouseWheelEventArgs& args)
{
    // By default, do nothing.
}

void Game::OnResize(ResizeEventArgs& args)
{
    m_Width = args.Width;
    m_Height = args.Height;
}

void Game::OnWindowDestroy()
{
    // If the Window which we are registered to is 
    // destroyed, then any resources which are associated 
    // to the window must be released.
    UnloadContent();
}

