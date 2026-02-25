add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

option("tests")
    set_default(false)
    set_showmenu(true)
    set_description("Enable tests")
option_end()

add_requires("levilamina 1.9.5", { configs = { target_type = get_config("target_type") } })
add_requires("levibuildscript 0.5.0")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("iListenAttentively")
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/W4",
        "/w44265",
        "/w44289",
        "/w44296",
        "/w45263",
        "/w44738",
        "/w45204",
        "/Ob3",
        "/Zo-"
    )
    add_shflags(
        "/DELAYLOAD:bedrock_runtime.dll"
    )
    add_defines(
        "NOMINMAX", 
        "UNICODE",
        "ILA_EXPORT",
        "_HAS_CXX23=1",
        ( is_config("target_type", "server") and "LL_PLAT_S" or "LL_PLAT_C" ),
        ( is_mode("debug") and "ILA_DEBUG" or "ILA_RELEASE" )
    )
    set_optimize("aggressive")
    set_configdir("$(builddir)/config")
    add_files("src/**.rc")
    add_includedirs("$(builddir)/config")
    add_packages("levilamina")
    set_exceptions("none")
    set_kind("shared")
    set_languages("cxx20")
    set_symbols("debug")
    if is_mode("release") then
        set_strip("all")
    else
        remove_files("src/common/ila/core/SymbolProvider.cpp")
    end
    local XorKey = math.random(1, 255)

    add_files("src/common/**.cpp")
    add_includedirs("src/common")
    add_headerfiles("src/common/ila/**.h", "src/common/ila/**.hpp")
    add_configfiles("src/common/(**.h.in)")

    local target_type = get_config("target_type") or "server"
    add_files("src/" .. target_type .. "/**.cpp")
    add_includedirs("src/" .. target_type)
    add_headerfiles("src/" .. target_type .. "/ila/**.h", "src/" .. target_type .. "/ila/**.hpp")
    add_configfiles("src/" .. target_type .. "/(**.h.in)")

    if has_config("tests") then
        add_defines("ILA_TESTS")
        add_files("src-test/common/**.cpp")
        add_includedirs("src-test/common/")
        add_headerfiles("src-test/common/**.h")

        add_files("src-test/" .. target_type .. "/**.cpp")
        add_includedirs("src-test/" .. target_type .. "/")
        add_headerfiles("src-test/" .. target_type .. "/**.h")
    end

    on_load(function (target)
        local version_info = import("scripts.get-version-info", { rootdir = os.projectdir() }).get_version_info()
        target:set("configvar", "ILA_VERSION_MAJOR", version_info.major)
        target:set("configvar", "ILA_VERSION_MINOR", version_info.minor)
        target:set("configvar", "ILA_VERSION_PATCH", version_info.patch)
        if version_info.prerelease then
            target:set("configvar", "ILA_VERSION_PRERELEASE", version_info.prerelease)
        end
    end)

    if is_mode("release") then
        before_build(function (target) 
            io.gsub(
                path.join(
                    os.projectdir(),
                    "src",
                    "common",
                    "ila",
                    "core",
                    "SymbolProvider.cpp"
                ),
                "constexpr uint8_t mXorKey = %d+;",
                "constexpr uint8_t mXorKey = " .. XorKey .. ";"
            )
        end)
    end

    before_link(function(target)
        import("lib.detect.find_file")
        import("core.project.config")

        -- 修复莫名其妙的环境变量缺失导致的链接失败
        os.addenvs(target:pkgenvs())

        local libdir = path.join(config.builddir(), ".prelink", "lib")
        if os.exists(libdir) then os.rm(libdir) end
        os.mkdir(libdir)

        local data = assert(find_file("bedrock_runtime_data", {"$(env PATH)"}), "Cannot find bedrock_runtime_data")
        local link = assert(find_file("prelink.exe", {"$(env PATH)"}), "Cannot find prelink.exe")

        os.runv(link, {
            string.format("%s-%s-%s", get_config("target_type"), target:plat(), target:arch()),
            path.join(config.builddir(), ".prelink"),
            data,
            table.unpack(target:objectfiles())
        })

        target:add("linkdirs", libdir)
        target:add("links", "bedrock_runtime_api")
    end)

    after_build(function (target)
        local output_dir = path.join(os.projectdir(), "bin", "dll", target:name())

        os.rm(output_dir)

        os.vcp(target:targetfile(), format("%s/", output_dir))
        os.vcp(target:symbolfile(), format("%s/../../pdb/", output_dir))
        os.run(path.join(os.projectdir(), "tools", "iLitePDB.exe"))

        if is_mode("release") then
            os.runv(
                "python.exe",
                {
                    path.join(os.projectdir(), "tools", "iEncryptImports.py"),
                    path.join(os.projectdir(), "bin", "dll", target:name(), path.filename(target:targetfile())),
                    XorKey
                }
            )
        end

        import("scripts.generate-manifest", { rootdir = os.projectdir() }).generate_manifest(
            format("%s/manifest.json", output_dir),
            {
                name = "iListenAttentively",
                entry = "iListenAttentively.dll",
                version = import("scripts.get-version-info", { rootdir = os.projectdir() }).get_version_info().version_str,
                author = "MiracleForest",
                description = "iListenAttentively is a rich and modern LeviLamina Minecraft event library!",
                passive = not get_config("tests") and is_mode("release"),
                platform = get_config("target_type")
            }
        )
    end)

    on_package(function (target)
        local output_dir = path.join(os.projectdir(), "bin", "sdk")

        os.rm(output_dir)

        for _, header_file in ipairs(target:headerfiles()) do
            if not header_file:endswith(".i.h") then
                os.vcp(
                    header_file,
                    path.join(
                        output_dir,
                        "include",
                        (path.relative(header_file, os.projectdir()):gsub("^[^/\\]+[/\\][^/\\]+[/\\]", ""))
                    )
                )
            end
        end

        for _, source_file in ipairs(target:sourcefiles()) do
            if source_file:endswith(".open.cpp") then
                os.vcp(
                    source_file,
                    path.join(
                        output_dir,
                        "src",
                        ((path.relative(source_file, os.projectdir()):gsub("^[^/\\]+[/\\]", "")):gsub("%.open%.cpp$", ".cpp"))
                    )
                )
            end
        end

        local target_implib = target:artifactfile("implib")
        if target_implib and os.isfile(target_implib) then
            os.vcp(target_implib, format("%s/lib/", output_dir))
        end
    end)