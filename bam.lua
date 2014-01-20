function Intermediate_Output(settings, input)
	return "objs/" .. string.sub(PathBase(input), string.len("src/")+1)
end

settings = NewSettings()
settings.optimize = 1
settings.debug = 0
settings.cc.Output = Intermediate_Output

settings.cc.includes:Add("src")
settings.cc.includes:Add("include")

objs = Compile(settings, CollectRecursive("src/*.cpp"))

if family == "windows" then
	settings.link.libs:Add("lib/sdl/SDL")
	settings.link.libs:Add("lib/sdl/SDLmain")
else
	settings.link.libs:Add("SDL")
end

exe = Link(settings, "SuperPoussin", objs)
