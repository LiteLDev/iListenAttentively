function generate_manifest(output_file, info)
    local manifest = {
        entry = "",
        name = "",
        type = "native"
    }

    for k, v in pairs(info) do  manifest[k] = v end

    local validators = {
        -- 必需字段验证
        name = function(v) return type(v) == "string" and v ~= "" end,
        entry = function(v) return type(v) == "string" and v ~= "" end,
        type = function(v) return type(v) == "string" and v ~= "" end,

        -- 可选字段验证
        passive = function(v) return v == nil or type(v) == "boolean" end,
        version = function(v) return v == nil or type(v) == "string" end,
        author = function(v) return v == nil or type(v) == "string" end,
        description = function(v) return v == nil or type(v) == "string" end,
        platform = function(v) 
            if v == nil then return true end
            if type(v) ~= "string" then return false end
            return v == "universal" or v == "client" or v == "server"
        end,

        -- 复杂类型验证
        extraInfo = function(v)
            if v == nil then return true end
            if type(v) ~= "table" then return false end
            for k, val in pairs(v) do
                if type(k) ~= "string" or type(val) ~= "string" then
                    return false
                end
            end
            return true
        end,

        dependencies = function (v) 
            if deps == nil then return true end
            if type(deps) ~= "table" then return false end

            for _, dep in ipairs(deps) do
                if type(dep) ~= "table" or type(dep.name) ~= "string" then
                    return false
                end
                if dep.version ~= nil and type(dep.version) ~= "string" then
                    return false
                end
            end
            return true
        end
    }
    validators.optionalDependencies = validators.dependencies
    validators.conflicts = validators.dependencies
    validators.loadBefore = validators.dependencies

    for field, validator in pairs(validators) do
        if not validator(manifest[field]) then
            os.raise("Manifest " .. field .. " is missing or invalid")
        end
    end

    -- 没格式化不好看
    -- import("core.base.json")
    -- json.savefile(output_file, manifest)

    io.writefile(output_file, import("scripts.json", { rootdir = os.projectdir() }).encode(manifest, 4))
end