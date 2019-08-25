target = 'NSxfer'

files = Split("""
	gui.c
	main.c
	queue.c
	thread.c
	utils.c
""")

resources = Split("""
	resource.rc
""")

libs = Split("""
	kernel32
	advapi32
	ole32
	uuid
	user32
	shlwapi
	version
	wininet
""")

examples = Split("""
	Test/NSxfer-Test.nsi
	Test/NSxfer-Test-build.bat
""")

docs = Split("""
	NSxfer.Readme.txt
""")

Import('BuildPlugin')

BuildPlugin(target, files, libs, examples, docs, res = resources)