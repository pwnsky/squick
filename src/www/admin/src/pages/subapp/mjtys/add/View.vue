<template>
  <a-card :body-style="{ padding: '24px 32px' }" :bordered="false">
    <a-form :form="form">
      <a-form-item
        :label="`标题`"
        :labelCol="{ span: 7 }"
        :wrapperCol="{ span: 10 }"
      >
        <a-input
          placeholder="视频标题"
          v-decorator="[
            'title',
            {
              rules: [
                { required: true, message: '视频标题', whitespace: true },
              ],
            },
          ]"
        />
      </a-form-item>

      <a-form-item
        :label="`描述`"
        :labelCol="{ span: 7 }"
        :wrapperCol="{ span: 10 }"
      >
        <a-textarea
          rows="4"
          placeholder="视频描述"
          v-decorator="[
            'intro',
            {
              rules: [
                { required: true, message: '视频描述', whitespace: true },
              ],
            },
          ]"
        />
      </a-form-item>

      <a-form-item
        :label="`视频预览图片`"
        :labelCol="{ span: 7 }"
        :wrapperCol="{ span: 10 }"
      >
        <a-upload
          accept=".jpg"
          :previewVisible="false"
          :multiple="false"
          :customRequest="uploadImage"
        >
          <a-button class="upload-btn" type="primary">
            <a-icon type="upload" />
            {{ uploadImageButtonText }}
          </a-button>
        </a-upload>

        <img
          :src="uploadImageUrl"
          v-show="isUploadImage == true"
          width="384"
          height="216"
        />
        <p>{{ uploadImageText }}</p>
      </a-form-item>

      <a-form-item
        :label="`视频文件`"
        :labelCol="{ span: 7 }"
        :wrapperCol="{ span: 10 }"
      >
        <a-upload
          accept=".mp4"
          :previewVisible="false"
          :multiple="false"
          :customRequest="uploadVideo"
        >
          <a-button class="upload-btn" type="primary">
            <a-icon type="upload" />
            {{ uploadVideoButtonText }}
          </a-button>
        </a-upload>

        <p>{{ uploadVideoText }}</p>
      </a-form-item>

      <a-form-item
        :label="`类别`"
        :labelCol="{ span: 7 }"
        :wrapperCol="{ span: 10 }"
        :required="false"
        :help="`'类别`"
      >
        <a-radio-group
          v-model="value"
          v-decorator="['type', { rules: [{ required: true }] }]"
        >
          <a-radio :value="1">{{ `默认` }}</a-radio>
        </a-radio-group>
      </a-form-item>

      <a-form-item
        style="margin-top: 24px"
        :wrapperCol="{ span: 10, offset: 7 }"
      >
        <a-button type="primary" @click="submit">{{ `提交` }}</a-button>
      </a-form-item>
    </a-form>
  </a-card>
</template>

<script>
import { handleResourcesAdd } from "@/services/common/resources";
import { handleVideoAdd, handleVideoUpdate } from "@/services/subapp/mjtys";
export default {
  name: "View",
  data() {
    return {
      value: 1,
      form: this.$form.createForm(this),
      isUploadImage: false,
      isUploadVideo: false,
      uploadImageButtonText: "上传图片",
      uploadVideoButtonText: "上传视频",
      uploadImageText:
        "请上传分辨率为 768 x 432 格式为 jpg 的图片，图片大小不得超过 1M",
      uploadVideoText:
        "请上传分辨率为 1920 x 1080 格式为 mp4 的视频，且采用PR导出的目标比特率不高于5",
      uploadImageUrl: "",
      uploadVideoUrl: "",
      isEdit : false,
    };
  },
  beforeMount() {
    // 初始化函数
    //this.number_of_itmes_per_page = 1;
    //this.refrashView(0); // 默认是获取未注册的
  },
  computed: {
    desc() {
      return this.$t("pageDesc");
    },
  },
  methods: {
    uploadImage(e) {
      const formData = new FormData();
      formData.append("file", e.file);

      let configs = {
        onUploadProgress: (progressEvent) => {
          if (progressEvent.lengthComputable) {
            //是否存在进度
            var percentCompleted = Math.round(
              (progressEvent.loaded * 100) / progressEvent.total
            );
            this.uploadImageText = "上传进度为: " + percentCompleted + "%";
          }
        },
      };

      //alert("上传图片")
      handleResourcesAdd(formData, configs)
        .then((res) => {
          const rep = res.data;
          if (rep.code == 200) {
            this.$message.success("上传成功", 3);
            this.uploadImageUrl = rep.data.url;
            this.uploadImageText = "已经上传" + rep.data.url;
            this.uploadImageButtonText = "重新上传";
            this.isUploadImage = true;
            //this.$refs.md.$img2Url(pos, rep.data.url);
          } else {
            this.$message.error("上传失败" + rep.msg, 3);
          }
        })
        .catch((err) => {
          this.$message.error("上传失败: " + err, 3);
          console.log(err);
        });
    },
    uploadVideo(e) {
      const formData = new FormData();
      formData.append("file", e.file);

      let configs = {
        onUploadProgress: (progressEvent) => {
          if (progressEvent.lengthComputable) {
            //是否存在进度
            var percentCompleted = Math.round(
              (progressEvent.loaded * 100) / progressEvent.total
            );
            this.uploadVideoText =
              "上传中: " +
              (progressEvent.loaded / (1024 * 1024)).toFixed(2) +
              "MB / " +
              (progressEvent.total / (1024 * 1024)).toFixed(2) +
              "MB    进度 " +
              percentCompleted +
              "%";
          }
        },
      };

      handleResourcesAdd(formData, configs)
        .then((res) => {
          const rep = res.data;
          if (rep.code == 200) {
            this.$message.success("上传成功", 3);
            this.uploadVideoUrl = rep.data.url;
            this.uploadVideoText = "已经上传" + rep.data.url;
            this.uploadVideoButtonText = "重新上传";
            this.isUploadVideo = true;
            //this.$refs.md.$img2Url(pos, rep.data.url);
          } else {
            this.$message.error("上传失败" + rep.msg, 3);
          }
        })
        .catch((err) => {
          this.$message.error("上传失败: " + err, 3);
          console.log(err);
        });
    },
    submit() {
      //alert("sss");
      this.form.validateFields((err) => {
        if (!err && this.isUploadImage && this.isUploadVideo) {
          

          //console.log(req);
          const req = {
            title: this.form.getFieldValue("title"),
            intro: this.form.getFieldValue("intro"),
            type: this.form.getFieldValue("type"),
            image_url: this.uploadImageUrl,
            video_url: this.uploadVideoUrl,
          };
          if (this.isEdit == false) {
            

            handleVideoAdd(req).then((res) => {
              const updateRes = res.data;
              if (updateRes.code === 200) {
                this.$message.success("发布成功", 3);
                this.$router.push("/sub_app/mjtys/list");
              } else if (updateRes.code === 400) {
                this.$message.error("发布失败: " + updateRes.msg, 3);
              }
            }).catch(err => {
                    this.$message.error('发布失败' + err, 3)
                    console.log(err)
                })
          } else {
            req["id"] = parseInt(this.editModeId);
            handleVideoUpdate(req).then((res) => {
              const updateRes = res.data;
              if (updateRes.code === 200) {
                alert("更新成功");
                this.$message.success("更新成功", 3);
              } else if (updateRes.code === 400) {
                this.$message.error("更新失败: " + updateRes.msg, 3);
              }
            }).catch(err => {
                    this.$message.error('更新失败' + err, 3)
                    console.log(err)
                })
          }
        }
      });
    },
  },
};
</script>

<style scoped>
</style>
