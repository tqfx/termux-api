-- set project name
set_project("termux-api")

-- set xmake minimum version
set_xmakever("2.5.0")

-- set project version
set_version("0.1.0", {build = "%Y%m%d%H%M"})

-- set language: c11 c++17
set_languages("c11", "c++17")

-- option: warnings
option("warnings")
    set_default(false)
    set_showmenu(true)
    set_description("Enable or disable warnings")
option_end()

if has_config("warnings") then
    -- set warning everything
    set_warnings("everything")
    -- disable some compiler errors
    add_cxflags("-Wno-reserved-identifier", "-Wno-used-but-marked-unused")
    add_cflags("-Wno-declaration-after-statement")
    add_cxxflags("-Wno-c++98-compat-pedantic")
end

-- add build modes
add_rules("mode.check", "mode.debug", "mode.release")
if is_mode("check") then
    set_policy("check.auto_ignore_flags", false)
    local flags = {
        "-fsanitize=address,undefined",
        "-fsanitize-recover=address",
        "-fno-omit-frame-pointer",
        "-fsanitize=leak",
    }
    add_cxflags(flags)
    add_ldflags(flags)
end

target("termux_api")
    -- make as a shared library
    set_kind("shared")
    -- add defines
    add_defines("_GNU_SOURCE=1")
    -- add link libraries
    add_links("termux-api", "jansson", {public = true})
    -- add include directories
    add_includedirs("include", {public = true})
    -- add the header files for installing
    add_headerfiles("include/(**.h)")
    -- add the common source files
    add_files("src/**.c")
target_end()

-- include test sources
includes("test")
