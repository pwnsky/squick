<template>

    

    <div>

        <div class="markdown">
        <div class="container">
            <mavon-editor v-model="content" ref="md" @imgAdd="$imgAdd" @change="change" style="min-height: 600px"/>
            
        </div>
        </div>

     <a-card :bordered="false">
      <div style="display: flex; flex-wrap: wrap">
          <head-info :title="title" :content="number_of_itmes" :bordered="true"/>
      </div>
    </a-card>
    
    <a-card :bordered="false" class="submit-form">
    
    <a-form :form="form">
    <a-form-item
          :label="$t('文章标题')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="文章标题"
            v-decorator="['title', {rules: [{ required: true, message: '文章标题', whitespace: true}]}]"
        />
      </a-form-item>

      <a-form-item
          :label="$t('分类')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="分类"
            v-decorator="['categories', {rules: [{ required: true, message: '文章分类', whitespace: true}]}]"
        />
      </a-form-item>

    <a-form-item
          :label="$t('标签')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="标签"
            v-decorator="['tag', {rules: [{ required: true, message: '文章标签', whitespace: true}]}]"
        />
      </a-form-item>

      <a-form-item
          :label="$t('作者')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="作者"
            v-decorator="['author', {rules: [{ required: true, message: '作者姓名', whitespace: true}]}]"
        />
      </a-form-item>
    </a-form>

    <center>
        <a-button   type="primary" @click="submit">发布</a-button>
    </center>
    
   </a-card>

    </div>
    
</template>

<script>

function getQueryVariable(variable)
{
    var urlA = window.location.toString().split('?')
    if(urlA.length < 2) {
                
        return false
    }
    var query = urlA[1];
    var vars = query.split("&");
    for (var i=0;i<vars.length;i++) {
        var pair = vars[i].split("=");
        if(pair[0] == variable){return pair[1];}
    }
    return(false);
}
    import { mavonEditor } from 'mavon-editor'
    import 'mavon-editor/dist/css/index.css'
    import {handleResourcesAdd} from "@/services/common/resources";
    import {handleArticalAdd, handleArticalGet, handleArticalUpdate} from "@/services/subapp/zyzx";
    export default {
        name: "",
        props: [],
        components: {
            mavonEditor,
        },
        

        data() {
            return {
                content:'',
                html:'',
                configs: {},
                form: this.$form.createForm(this),
                isEdit: false,
                editModeId: 0
            }
        },
        beforeMount() { // 初始化函数
            if(getQueryVariable('type') != false && getQueryVariable('id') != null) {
                this.isEdit = true;
                this.editModeId = getQueryVariable('id')
                //alert('edit mode')

                handleArticalGet(this.editModeId).then(res => {
                        const updateRes = res.data
                        if (updateRes.code === 200) {
                            this.$message.success('获取成功', 3)

                            this.content = updateRes.data.content
                            this.form.setFieldsValue({
                                title: updateRes.data.title,
                                tag: updateRes.data.tag,
                                categories: updateRes.data.categories,
                                author: updateRes.data.author,
                            })
                            //alert(updateRes)
                            //this.$router.push('/devices_manager/sub_app/zyzx/list')
                        } else if (updateRes.code === 400) {
                            
                            this.$message.error('获取失败: ' + updateRes.msg, 3)
                        }
                    })
            }           
            
        },
        methods: {
            // 将图片上传到服务器，返回地址替换到md中
            $imgAdd(pos, $file){
                let formdata = new FormData();
                if($file == null) {
                    return
                }

                formdata.append('file', $file)

                 handleResourcesAdd(formdata).then(res => {
                    const rep = res.data
                    if(rep.code == 200) {
                        this.$message.success('上传成功', 3)
                        this.$refs.md.$img2Url(pos, rep.data.url);
                    }else {
                        this.$message.error('上传失败' + rep.msg , 3)
                    }
                    
                 }).catch(err => {
                    this.$message.error('上传失败' + err, 3)
                    console.log(err)
                })
            },
            // 所有操作都会被解析重新渲染
            change(value, render){
                // render 为 markdown 解析后的结果[html]
                this.html = render;
            },
            // 提交
            submit(){
                this.form.validateFields((err) => {
                    if (!err) {
                        //console.log(this.content);
                    // console.log(this.html);
                    //this.$message.success('提交成功，已打印至控制台！');
                    const artical = {
                        title : this.form.getFieldValue('title'),
                        tag : this.form.getFieldValue('tag'),
                        categories : this.form.getFieldValue('categories'),
                        author : this.form.getFieldValue('author'),
                        content : this.content,
                    }
                    if(this.isEdit == false) {
                        handleArticalAdd(artical).then(res => {
                            const updateRes = res.data
                            if (updateRes.code === 200) {
                                this.$message.success('发布成功', 3)
                                this.$router.push('/sub_app/zyzx/list')
                            } else if (updateRes.code === 400) {
                                this.$message.error('发布失败: ' + updateRes.msg, 3)
                                
                            }
                        })
                    }else {
                        artical["id"] = parseInt(this.editModeId);
                        handleArticalUpdate(artical).then(res => {
                            const updateRes = res.data
                            if (updateRes.code === 200) {
                                alert('更新成功')
                                this.$message.success('更新成功', 3)
                                //this.$router.push('/devices_manager/sub_app/zyzx/list')
                            } else if (updateRes.code === 400) {
                                this.$message.error('发布失败: ' + updateRes.msg, 3)
                            }
                        })
                    }

                    }
                    
                    
                 })
                
            },
        },
        mounted() {

        }
    }
</script>
