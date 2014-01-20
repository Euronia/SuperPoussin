for i in Dir.new("scripts").entries
	puts i if i != "." and i != ".."
end
