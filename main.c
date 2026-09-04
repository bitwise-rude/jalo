#include "jalo.h"

JaloOutput home (HTTP_Request rq) {
    return jalo_string_output("Hello To the people of internet, how the fuck are you guys?");
}

JaloOutput logo(HTTP_Request rq){
    return jalo_file_output(rq.path+1); // Don't want the / from first
}

JaloOutput test (HTTP_Request rq) {
    return jalo_render("test.html");
}

int main() {
    JaloServe js = jalo_init();

    jalo_register(&js,"/test", test);
    jalo_register(&js,"/assets/*", logo);

    jalo_run(&js, 8080);
    jalo_deinit(&js);

    return 0;
}
