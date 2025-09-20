LaunchBrowser()

function OnHttpRequest()
    local path = GetPath()
    if path == '/web' or path == '/web/' then
        ServeAsset("/web/index.html")
    else
        Route()
    end
end
