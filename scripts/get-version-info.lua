_cached_version_info = nil

function get_version_info()
    if not _cached_version_info then
        local major, minor, patch, suffix = os.iorun("git describe --tags --abbrev=0 --always"):match("v(%d+)%.(%d+)%.(%d+)(.*)")
        if not major then major, minor, patch = 0, 0, 0 end

        local prerelease
        if suffix then
            if suffix then suffix = suffix:match("-(.*)") end
            if suffix then suffix = suffix:gsub("\n", "") end
            if suffix then prerelease = suffix end
        end

        local commit_hash = os.iorun("git rev-parse --short HEAD"):gsub("\n", "")
        _cached_version_info = {
            major = major,
            minor = minor,
            patch = patch,
            prerelease = prerelease,
            commit_hash = commit_hash,
            version_str = major .. "." .. minor .. "." .. patch .. (prerelease and ("-" .. prerelease) or "") .. "+" .. commit_hash
        }
    end
    return _cached_version_info
end