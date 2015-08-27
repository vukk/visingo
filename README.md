# Visingo

This is a desktop app for creating visualizations directly from answer set programming solvers.
The development of interactive portions of Visingo is currently halted, but
playback visualizations are available with the
[Visingo-Web](https://github.com/vukk/visingo-web) project!

See <https://github.com/vukk/visingo-web> and <http://users.ics.aalto.fi/kuuranne/visingo-web/>.

## Subprojects

Qt pods are used as git submodules.
visingo-web is a git submodule also <https://github.com/vukk/visingo-web>.

Remember to run `git submodule init` and `git submodule update` to initialize
the submodules when cloning this repository.

## Ideas for continuing the project

- Create a more lightweight approach, use a Lua script and WebSockets to expose
  clingo's API and connect to it from Visingo-Web.

## Some notes

- We obviously can't refer to all answer sets when visualizing interactively,
  only to previous ones. To expose the previous answer sets, we would require
	some sort of postprocessing program/script/pipeline. More importantly this
	postprocessing shouldn't disable user's ability to steer the solving process
	interactively.
