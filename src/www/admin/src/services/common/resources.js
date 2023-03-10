import {METHOD, request} from "@/utils/request";
import {COMMON_RESOURCES_ADD} from "@/services/api";
//import {getCookie} from "@/utils/util";

export async function handleResourcesAdd(data, config) {
    return request(COMMON_RESOURCES_ADD, METHOD.POST, data, config)
}
