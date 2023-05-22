cmake ./

IF %ERRORLEVEL% NEQ 0 (
    PAUSE
) ELSE (
    START DirectX12-Sandbox.sln
)
