#include "../jalo.h"

JaloServe js;

JaloOutput home(HTTP_Request hr) {
    return jalo_render_template(&js,"./templates/index.html");
}

JaloOutput assets(HTTP_Request hr) {
    return jalo_file_output(hr.path+1);
}

JaloOutput blog(HTTP_Request hr) {
    return jalo_string_output("Hello");
}

int main() {
    js = jalo_init();

    jalo_register(&js, "/", home);
    jalo_register(&js, "/assets/*", assets);
    jalo_register(&js, "/blogs/*", blog);

    jalo_run(&js, 8080);
    jalo_deinit(&js);
}

