/**
 * 客户端设备
 */
 import {METHOD, request} from "@/utils/request";
 import {CLIENT_DEVICE_GET} from "@/services/api";
 //import {getCookie} from "@/utils/util";
 
 export async function handleGetClientDeviceInfo(data) {
     return request(CLIENT_DEVICE_GET, METHOD.POST, data)
 }