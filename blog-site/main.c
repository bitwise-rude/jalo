#include <jalo.h>

JaloServe js;

JaloOutput home(HTTP_Request hr) {
    jalo_execute_file(&js, "./lua/main.lua");
    return jalo_render_template(&js, "templates/index.html");
}

JaloOutput favicon(HTTP_Request hr){
    return jalo_file_output("./static/logo");
}

JaloOutput assets(HTTP_Request hr){
    return jalo_file_output(hr.path+1);
}

JaloOutput blogs_assets(HTTP_Request hr){
    return jalo_file_output(hr.path+7);
}

JaloOutput blogs(HTTP_Request hr){
    return jalo_render_template(&js,hr.path+1);
}

int main(){
    js = jalo_init();

    jalo_register(&js, "/", home);
    jalo_register(&js, "/favicon.ico", favicon);
    jalo_register(&js, "/static/*", assets);
    jalo_register(&js, "/blogs/static/*", blogs_assets);
    jalo_register(&js, "/blogs/*", blogs);

    jalo_run(&js,8000);
    jalo_deinit(&js);
}
