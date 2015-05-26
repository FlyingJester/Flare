# Flare
A Simple and Effective Code and Text Editor

### What is Flare?

Flare is a text and code editor. It aims to be a simple and useful editor for plain-text and simple markup text files, such as source code, markdown, and non- wysiswyg HTML.

### Why use Flare?

Flare is for those who want a graphical text editor, but do not want the complexity, overhead, or bugginess of editors such as Code::Blocks, CodeLite, XCode, or Visual Studio. If you like your editors featureful, this isn't the editor for you. Flare is intended to be graphical and capable, but also lightweight, stable, and simple.

On Unix platforms, many graphical code editors require a large widget toolkit, such as GTK or wxWidgets. Flare uses FLTK, which is a much smaller tookit. Its small size even allows for reasonably compiling Flare with FLTK statically linked. On older or less capable machines, FLTK also outperforms wxWidgets, Qt, and GTK. This can be especially pronounced on machines running outdated or alternative X Servers, such as the old Sun X Server.

### Why Make a New Editor?

Flare was started by Martin McDonough to replace CodeLite and Code::Blocks. Both are fairly good IDEs, but they are both rather buggy. Code::Blocks has been seen devouring files without a trace (!), and CodeLite has a few very annoying bugs, such as double cursors appearing from time to time, or being stuck entering text in a tab that is not active.

Other features of these and other editors are not considered a useful part of a code editor by Flare authors, such as project files that for compiling sources, or intellisense-like features. If you can't live without intellisense or IDE compilation, Flare isn't for you. It's meant to be minimalistic, focusing only on editing files efficiently and effectively. This does not include advanced source analysis or compiler invocation.

This does not mean that Flare is not intended to have more specialized editors as well as the default text/code editor.