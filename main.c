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
