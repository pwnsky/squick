
function isMobile() {
    if(navigator.userAgent.match(/Android/i) || navigator.userAgent.match(/iPhone|iPad|iPod/i) ) {
        return true
    }else {
        return false
    }
}


function ani(element) {
    VanillaTilt.init(document.querySelector(element), {
    max: 25,
    speed: 400,
    "max-glare": 1,
    gyroscope: true,
    });
    //It also supports NodeList
    VanillaTilt.init(document.querySelectorAll(element));
}

// 重新获取验证码图片
function change_captcha() {
        $.get("/api/pub/captcha/init",function(data, status){
        window.captcha_id = data.data
        document.getElementById('captcha_image').src="/api/pub/captcha/image?id=" + window.captcha_id
    }, "json");
}


function init() {
    change_captcha() // 获取验证码
}

init()

function login() {
    if($("#input_account").val().length < 6) {
        $("#error_notice").text("手机号或邮箱格式错误")
        return
    }
    
    if($("#input_password").val().length < 6  || $("#input_password").val().length > 32) {
        $("#error_notice").text("密码长度必须在 [6, 32] 区间")
        return
    }
    
    if($("#input_captcha").val().length != 5) {
        $("#error_notice").text("验证码格式错误")
        return
    }
    
    
    $(".login-panel").fadeOut(2000);
    $(".box-loading").fadeIn(2000);
    
    function InputLoginDataJson() {
        var input_data_json = {	
            "mobile":$("#input_account").val(), 
            "passwd":$("#input_password").val(),
            "captcha_code":$("#input_captcha").val(),
            "captcha_id":window.captcha_id,
        }
        return input_data_json
    }
   
   $.ajax({
    type: "POST",
    url: "/api/pub/sign_in/mobile_passwd",
    contentType: "application/json; charset=utf-8",
    data: JSON.stringify(InputLoginDataJson()),
    dataType: "json",
    success: function (data) {
        if(data.code == 400) {
            setTimeout(function() {
                $("#error_notice").text("登录失败: " + data.msg)
                change_captcha();
            }, 3000 )
            
        }else if(data.code == 200) {
            $("#error_notice").text("登录成功")
        }

        $(".login-panel").fadeIn(2000)
        $(".box-loading").fadeOut(2000)
        
    },
    error: function (data) {
        $("#error_notice").text("未知错误")
    }
});
    
}

function register() {
    if($("#input_account").val().length < 6) {
        $("#error_notice").text("手机号错误")
        return
    }
    
    if($("#input_password").val().length < 6  || $("#input_password").val().length > 32) {
        $("#error_notice").text("密码长度必须在 [6, 32] 区间")
        return
    }
    
    if($("#input_password").val() != $("#input_password").val()) {
        $("#error_notice").text("两次输入密码不一致")
        return
    }
    
    if($("#input_captcha").val().length != 5) {
        $("#error_notice").text("验证码格式错误")
        return
    }
    
    
    $(".login-panel").fadeOut(2000);
    $(".box-loading").fadeIn(2000);
    
    function InputRegisterDataJson() {
        var input_data_json = {	
            "mobile":$("#input_account").val(), 
            "passwd":$("#input_password").val(),
            "captcha_code":$("#input_captcha").val(),
            "captcha_id":window.captcha_id,
        }
        return input_data_json
    }
   
   $.ajax({
    type: "POST",
    url: "/api/pub/sign_up/mobile_passwd",
    contentType: "application/json; charset=utf-8",
    data: JSON.stringify(InputRegisterDataJson()),
    dataType: "json",
    success: function (data) {
        if(data.code == 400) {
            setTimeout(function() {
                $("#error_notice").text("注册失败: " + data.msg)
                change_captcha();
            }, 3000 )
            
        }else if(data.code == 200) {
            $("#error_notice").text("注册成功")
        }
        
        $(".login-panel").fadeIn(2000)
        $(".box-loading").fadeOut(2000)
        
    },
    error: function (data) {
        $("#error_notice").text("未知错误")
    }
});
    
}


// 	setTimeout(function() {
//         alert("/api/pub/captcha/image?id=" + window.captcha_id)
// 	}, 2000)
