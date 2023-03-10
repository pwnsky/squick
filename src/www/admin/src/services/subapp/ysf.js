import {METHOD, request} from "@/utils/request";
import {SUBAPP_YSF_VIDEO_ADD, SUBAPP_YSF_VIDEO_UPDATE, SUBAPP_YSF_VIDEO_GETALL, SUBAPP_YSF_VIDEO_DELETE, SUBAPP_YSF_VIDEO_GET} from "@/services/api";
//import {getCookie} from "@/utils/util";

export async function handleVideoAdd(data) {
    return request(SUBAPP_YSF_VIDEO_ADD, METHOD.POST, data)
}

export async function handleVideoGetAll() {
    return request(SUBAPP_YSF_VIDEO_GETALL, METHOD.GET)
}

export async function handleVideoGet(id) {
    return request(SUBAPP_YSF_VIDEO_GET + "?id=" + id, METHOD.GET)
}

export async function handleVideoUpdate(data) {
    return request(SUBAPP_YSF_VIDEO_UPDATE, METHOD.POST, data)
}


export async function handleVideoDelete(id) {
    return request(SUBAPP_YSF_VIDEO_DELETE + "?id=" + id, METHOD.GET)
}
