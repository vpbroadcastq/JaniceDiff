


## Initial prompt
I am wiritng a (file) diffing application for use by programers (ex, Beyond Compare, WinMerge).  I know roughly what I want it to look like, I know it will be written in C++, and I know it will be a QT Widgets application.  I need help developing a spec, and then i need to break down the development process into managable tasks.  Development is going to be heavily ai-assisted, so tasks will need to be small enough that they can be completed by ai and then evaluated by me.  

I think we need to start with the spec.  Develop a skeleton spec for this gui application and guide me through filling it out.  Again, ai is going to do most of the development so it needs to be appropriately detailed.

### Response
What I need from you next (low friction)
Answer these decisions (short bullets are fine). I will then lock v1 scope and convert the spec into a first set of “AI-sized” tickets with acceptance criteria.
1)  v1 scope: file diff only, or file + folder?
2)  Edit/merge in v1: read-only or basic merge?
3)  Intra-line highlighting in v1: yes/no
4)  Platforms for v1: Windows + Linux (assumed), macOS yes/no
5)  Encoding support: UTF-8 only, or UTF-8 + UTF-16 + locale?
6)  Large file policy: warn above what size?

If you answer those, I will produce:
- A tightened “v1 spec” (still skeletal but unambiguous)
- A task breakdown (tickets) sized for AI implementation
- A suggested repo structure (src/app, src/core, tests, fixtures) consistent with the spec

### My response
1)  We need to be able to handle individual files as well as full folders (it needs to work with git difftool --dir-diff)
2)  Read-only for version 1
3)  Intra-line highlighting is not needed for v1
4)  Windows and linux.  No need to worry about Apple
5)  utf-8 is fine for v1.  Maybe in the future we will support otehr encodings, but probably not
6)  Let's not worry about implementing size limits.  I will do testing later on and add limits and warnings myself

### My response along with the draft spec
Note that we *DO* need Git integration in v1.  It needs to be able to detect modified files (tracked and untracked).  See point (7) under "In scope (v1)".  











