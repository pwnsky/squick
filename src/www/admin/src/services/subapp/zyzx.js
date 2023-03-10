import {METHOD, request} from "@/utils/request";
import {SUBAPP_ZYZX_ARTICAL_ADD, SUBAPP_ZYZX_ARTICAL_GET_LIST, SUBAPP_ZYZX_ARTICAL_GET, SUBAPP_ZYZX_ARTICAL_UPDATE, SUBAPP_ZYZX_ARTICAL_DELETE} from "@/services/api";
//import {getCookie} from "@/utils/util";

export async function handleArticalAdd(data) {
    return request(SUBAPP_ZYZX_ARTICAL_ADD, METHOD.POST, data)
}

export async function handleArticalGetList() {
    return request(SUBAPP_ZYZX_ARTICAL_GET_LIST, METHOD.GET)
}


export async function handleArticalGet(id) {
    return request(SUBAPP_ZYZX_ARTICAL_GET + "?id=" + id, METHOD.GET)
}

export async function handleArticalUpdate(data) {
    return request(SUBAPP_ZYZX_ARTICAL_UPDATE, METHOD.POST, data)
}


export async function handleArticalDelete(id) {
    return request(SUBAPP_ZYZX_ARTICAL_DELETE + "?id=" + id, METHOD.GET)
}
