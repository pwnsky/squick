/**
 * 网关处理
 */
import {METHOD, request} from "@/utils/request";
import {GATEWAY_GET, GATEWAY_UPDATE} from "@/services/api";
//import {getCookie} from "@/utils/util";

export async function handleUpdateGateway(gateway) {
    return request(GATEWAY_UPDATE, METHOD.POST, gateway)
}

export async function handleGetGateway() {
    return request(GATEWAY_GET, METHOD.GET)
}
