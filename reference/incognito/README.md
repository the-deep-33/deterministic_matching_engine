incognito_engine.cpp is a second reference engine. It was written by Claude
(Anthropic) in a separate chat that had only a written spec of the matching
rules and no access to this repository. It has not been edited by hand.

It is used only as a test oracle: the same inputs go through both reference
engines and their outputs are diffed. Its assumptions are listed at the top
of the file.

Build: g++ -std=c++20 -o incognito_engine incognito_engine.cpp