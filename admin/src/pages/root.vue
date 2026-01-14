<script lang="ts" setup>
import { httpPost, httpGet } from '../net/http';
import { useRouter } from 'vue-router'
const { push } = useRouter()
    httpGet("/api/auth_check", (rsp : any)=>{
        if (rsp.data.code == 0) {
            push({ name: 'dashboard' })
        }
    }, (err : any) => {
        if (err.response.status == 401) {
            console.log("not auth")
            push({ name: 'login' })
        }
    });
</script>