# mineval
Minimalistic remote nvim eval. I wrote this because:
* I want to have remote nvim eval
* I decline to use python (i.e. [neovim-remote](https://github.com/mhinz/neovim-remote))
* I hate software bloat and gazillions of dependencies

# Building
As simple as
```
make
```

# Usage examples
Request nvim api in json format:
```
# nvim --api-info | nvimeval
```

Fix annoying nvim behavior explained in [issue 4299](https://github.com/neovim/neovim/issues/4299):
```
# Add following to the end of your .bashrc:

function cd() {
    builtin cd "$@" &&
        test -n "$NVIM" &&
        mineval "nvim_command(':tcd $PWD')" > /dev/null
}
```

Prevent neovim being accidentally opened from neovim terminal:
```
# Add following to the end of your .bashrc:

function nvim() {
    test -n "$NVIM" &&
        mineval "nvim_command(':e $@')" > /dev/null ||
        $(which nvim) "$@"
}
```

Or just use it as a weird calculator:
```
# mineval '3+4'
[1,0,null,7]
```
