# jalo
<img src="./assets/logo_large.png" width=250>

A Clua (C + Lua) way of making backend for your websites.

> This is under development. It's not ready for making an actual websites besides testings.
> Any form of contribution is highly appreciated.

# How it works?
It should work surfacely like this:
1. You write in C, everything happens in the C environment.
2. You use Lua as a templating language. You can template it inside HTML or even write script outside of it.

# What has been achieved?
1. You can make blueprints through C and register them. 

`main.c`
```c
#include "jalo.h"

JaloServe js;

JaloOutput home(HTTP_Request hr) {
    return jalo_render_template(&js, "./assets/test.html");
}

int main() {
    js = jalo_init();

    jalo_execute_file(&js,"./assets/init.lua");

    jalo_register(&js, "/", home);

    jalo_run(&js, 8080);
    jalo_deinit(&js);
}
```
`JaloServe js;` serves as the main state variable. Making it global makes it easier to passaround through your endpoints.

- You initialize with `jalo_init()`. 
- You can register a blueprint with `jalo_register`. The first argument of this function and almost all the library function is the pointer to the `JaloServe` object/struct. Then it needs the route path, and a callback.
- The call back in this case the function `home` should accept the `HTTP_Request` struct as its argument, and always return a `JaloOutput`. Every Endpoint callback should follow this.
- After registering all the blueprint `jalo_run` will run it on a port you provide it. This is a blocking call and will block execution until you stop execution manually.
- `jalo_deinit` is supposed to free memory or close sockets. 
-`jalo_execute_file` takes a lua script and executes it. If you wish to execute scripts outside of html files, this is the way to do it. It keeps track of variables you made inside that script. You can call this function multiple times too.


The `init.lua` looks like this:

```lua
username = "Meyan"
email = "dareludum@gmail.com"
```
These are just simple variable declearation. Don't use `local` because I haven't implemented it with local, haha.

The `home` endpoint here returns by rendering a template through `jalo_render_template`. This will open the html document and render any lua code inside. However, there are other ways to return from an endpoint like - 
- `jalo_string_output` - where you just have to return a string.
- `jalo_file_output` - where you have to return a file.

Some Information:
- If you do not implement the home endpoint or the `/` endpoint, jalo will create a default one. 
- You can also make wildcard endpoints by doing:

```c
    jalo_register(&js, "/assets/*", assets);
```
This will send all of the assets/<anything> request to assets. You can retrieve information about this request through the arguemnt you recieve in the call back `HTTP_Request`. It has a `path` string which contains the actual endpoint requested. For example it maybe have `/assets/stuff.png`. Unlike in other frameworks, you are responsible for creating `static` folder yourself by this method.

# Contributing
Just send a PR, or file an issue. 

![Screenshot from Phase one](./assets/screenshot_p1.png) 

