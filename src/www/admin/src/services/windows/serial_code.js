/**
 * 序列号生成
 */
 import {METHOD, request} from "@/utils/request";
 import {SERIAL_CODE_GENERATE, SERIAL_CODE_GET} from "@/services/api";
 //import {getCookie} from "@/utils/util";
 
 export async function handleGetSerialCode(data) {
     return request(SERIAL_CODE_GET, METHOD.POST, data)
 }
 
 export async function handleGenerateSerialCode(data) {
     return request(SERIAL_CODE_GENERATE, METHOD.POST, data)
 }
 