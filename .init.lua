LaunchBrowser()

function ListFiles(dir)
    local files = GetZipPaths(dir)
    local output = {}
    for i, path in ipairs(files) do
        local clean = path:gsub("^" .. dir .. "/?", "")
        if clean ~= "" then
            table.insert(output, clean)
        end
    end
    return table.concat(output, "<br>")
end

function OnHttpRequest()
    local path = GetPath()
    if path == '/' or path == '' then
        ServeAsset("/web/index.html")
    elseif path:find('^/list/') then
        local listPath = path:sub(7)
        if listPath == '' then
            Write(ListFiles('/web'))
        else
            Write(ListFiles('/' .. listPath))
        end
    else
        Route()
    end
end
