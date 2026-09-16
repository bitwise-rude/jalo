#include <jalo.h>

JaloServe js;

JaloOutput home(HTTP_Request hr) {
    return jalo_render_template(&js, "templates/index.html");
}

int main(){
    js = jalo_init();

    jalo_register(&js, "/", home);

    jalo_run(&js,8000);
    jalo_deinit(&js);
}
