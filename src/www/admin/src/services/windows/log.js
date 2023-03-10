/**
 * 客户端日志
 */
 import {METHOD, request} from "@/utils/request";
 import {CLIENT_LOG_GET} from "@/services/api";
 //import {getCookie} from "@/utils/util";
 
 export async function handleGetClientLog(data) {
     return request(CLIENT_LOG_GET, METHOD.POST, data)
 }