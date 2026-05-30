class HalDialogWindow {
    constructor(bg,fg,bbg,bfg,pbg="#ffffff",pfg="#333333") {
        this.bg=bg;
        this.fg=fg;
        this.bbg=bbg;
        this.bfg=bfg;
        this.pbg=pbg;
        this.pfg=pfg;
        this.elem=document.createElement("div");
        this.elem.style="background-color:"+bg+";position:fixed;z-index:2147483647;max-width:500px;top:50%;left:50%;transform:translate(-50%,-50%);padding:5px;width:calc(100% - 10px);";
        this.elem.id="HalDialogWindow";
        this.elemBackground=document.createElement("div");
        this.elemBackground.id="HalDialogWindowBackground";
        this.elemBackground.style="background-color:rgba(0,0,0,0.5);z-index:2147483647;position:fixed;top:0;left:0;width:100%;height:100%;"
    }
    
    alert(text) {
        if (document.querySelector("#HalDialogWindow")===null) {
            this.elem.innerHTML=`<span style="width:100%;padding:5px;border:0;font-family:Roboto Condensed;display:block;color:`+this.fg+`;word-break:break-word;user-select:none;font-size:24px;">`+text+`</span>
<input type="button" value="Ок" style="background-color:`+this.bbg+`;color:`+this.bfg+`;border:0;border-radius:15px;font-size:24px;display:block;float:right;user-select:none;outline:none;width:fit-content;height:35px;cursor:pointer;">`;
            document.querySelector("body").appendChild(this.elemBackground);
            document.querySelector("body").appendChild(this.elem);
            return new Promise(function(resolve,reject) {
                var elem=document.querySelector("#HalDialogWindow");
                var elemBG=document.querySelector("#HalDialogWindowBackground");
                elem.children[1].onclick=function(event) {
                    elem.remove();
                    elemBG.remove();
                    resolve();
                };
            });
        }
        return false;
    }
    
    confirm(text) {
        if (document.querySelector("#HalDialogWindow")===null) {
            this.elem.innerHTML=`<span style="width:100%;padding:5px;border:0;font-family:Roboto Condensed;display:block;color:`+this.fg+`;word-break:break-word;user-select:none;font-size:24px;">`+text+`</span>
<input type="button" value="Ок" style="background-color:`+this.bbg+`;color:`+this.bfg+`;border:0;border-radius:15px;font-size:24px;display:block;float:right;user-select:none;outline:none;width:fit-content;height:35px;cursor:pointer;">
<input type="button" value="Отмена" style="background-color:`+this.bbg+`;color:`+this.bfg+`;border:0;border-radius:15px;font-size:24px;display:block;float:right;user-select:none;outline:none;width:fit-content;margin-right:5px;height:35px;cursor:pointer;">`;
            document.querySelector("body").appendChild(this.elemBackground);
            document.querySelector("body").appendChild(this.elem);
            return new Promise(function(resolve,reject) {
                var elem=document.querySelector("#HalDialogWindow");
                var elemBG=document.querySelector("#HalDialogWindowBackground");
                elem.children[2].onclick=function(event) {
                    elem.remove();
                    elemBG.remove();
                    resolve(false);
                };
                elem.children[1].onclick=function(event) {
                    elem.remove();
                    elemBG.remove();
                    resolve(true);
                };
            });
        }
        return false;
    }
    
    prompt(text,textButton="Ок",inputValue="",password=false) {
        if (document.querySelector("#HalDialogWindow")===null) {
            this.elem.innerHTML=`<span style="width:calc(100% - 10px);padding:5px;border:0;font-family:Roboto Condensed;display:block;color:`+this.fg+`;word-break:break-word;user-select:none;font-size:24px;">`+text+`</span>
<input type="`+(password===true?"password":"text")+`" value="`+inputValue+`" placeholder="" style="background-color:`+this.pbg+`;color:`+this.pfg+`;max-width:450px;width:calc(100% - 50px);margin:5px 25px 10px 25px;user-select:none;height:30px;outline:none;display:block;font-size:24px;font-family:Roboto Condensed;border:0;padding:5px;">
<input type="button" value="`+textButton+`" style="background-color:`+this.bbg+`;color:`+this.bfg+`;border:0;border-radius:15px;font-size:24px;display:block;float:right;user-select:none;outline:none;width:fit-content;height:35px;cursor:pointer;">
<input type="button" value="Отмена" style="background-color:`+this.bbg+`;color:`+this.bfg+`;border:0;border-radius:15px;font-size:24px;display:block;float:right;user-select:none;outline:none;width:fit-content;margin-right:5px;height:35px;cursor:pointer;">`;
            document.querySelector("body").appendChild(this.elemBackground);
            document.querySelector("body").appendChild(this.elem);
            return new Promise(function(resolve,reject) {
                var elem=document.querySelector("#HalDialogWindow");
                var elemBG=document.querySelector("#HalDialogWindowBackground");
                elem.children[3].onclick=function(event) {
                    elem.remove();
                    elemBG.remove();
                    resolve(false);
                };
                elem.children[2].onclick=function(event) {
                    elem.remove();
                    elemBG.remove();
                    resolve(elem.children[1].value);
                };
                
                elem.children[1].onkeydown=function(event) {
                    if (event.keyCode===13) {
                        event.preventDefault();
                        elem.remove();
                        elemBG.remove();
                        resolve(this.value);
                    }
                };
            });
        }
        return false;
    }
    
    select(text,options) {
        if (document.querySelector("#HalDialogWindow")===null) {
            var optionsText='';
            for (var i=0;i<options.length;i++) {
                optionsText+='<option value="'+options[i][1]+'">'+options[i][0]+'</option>';
            }
            this.elem.innerHTML=`<span style="width:calc(100% - 10px);padding:5px;border:0;font-family:Roboto Condensed;display:block;color:`+this.fg+`;word-break:break-word;user-select:none;font-size:24px;">`+text+`</span>
<select style="height:50px;font-family:Roboto Condensed;font-weight:bold;font-size:24px;color:#363636;border-radius:15px;border:0;display:block;padding:0 10px 0 10px;width:100%;">
    `+optionsText+`
</select>
<input type="button" value="Ок" style="background-color:`+this.bbg+`;color:`+this.bfg+`;border:0;border-radius:15px;font-size:24px;display:block;float:right;user-select:none;outline:none;width:fit-content;height:35px;cursor:pointer;">
<input type="button" value="Отмена" style="background-color:`+this.bbg+`;color:`+this.bfg+`;border:0;border-radius:15px;font-size:24px;display:block;float:right;user-select:none;outline:none;width:fit-content;margin-right:5px;height:35px;cursor:pointer;">`;
            document.querySelector("body").appendChild(this.elemBackground);
            document.querySelector("body").appendChild(this.elem);
            return new Promise(function(resolve,reject) {
                var elem=document.querySelector("#HalDialogWindow");
                var elemBG=document.querySelector("#HalDialogWindowBackground");
                elem.children[3].onclick=function(event) {
                    elem.remove();
                    elemBG.remove();
                    resolve(false);
                };
                elem.children[2].onclick=function(event) {
                    elem.remove();
                    elemBG.remove();
                    resolve(elem.children[1].value);
                };
            });
        }
        return false;
    }
    
    text(text,textButton="Ок",textValue="") {
        if (document.querySelector("#HalDialogWindow")===null) {
            this.elem.innerHTML=`<span style="width:calc(100% - 10px);padding:5px;border:0;font-family:Roboto Condensed;display:block;color:`+this.fg+`;word-break:break-word;user-select:none;font-size:24px;">`+text+`</span>
<textarea type="text" placeholder="" style="background-color:`+this.pbg+`;color:`+this.pfg+`;max-width:480px;width:calc(100% - 20px);margin:5px 10px 10px 10px;user-select:none;height:300px;outline:none;display:block;font-size:24px;font-family:Roboto Condensed;border:0;padding:5px;resize:none;">`+textValue+`</textarea>
<input type="button" value="`+textButton+`" style="background-color:`+this.bbg+`;color:`+this.bfg+`;border:0;border-radius:15px;font-size:24px;display:block;float:right;user-select:none;outline:none;width:fit-content;height:35px;cursor:pointer;">
<input type="button" value="Отмена" style="background-color:`+this.bbg+`;color:`+this.bfg+`;border:0;border-radius:15px;font-size:24px;display:block;float:right;user-select:none;outline:none;width:fit-content;margin-right:5px;height:35px;cursor:pointer;">`;
            document.querySelector("body").appendChild(this.elemBackground);
            document.querySelector("body").appendChild(this.elem);
            return new Promise(function(resolve,reject) {
                var elem=document.querySelector("#HalDialogWindow");
                var elemBG=document.querySelector("#HalDialogWindowBackground");
                elem.children[3].onclick=function(event) {
                    elem.remove();
                    elemBG.remove();
                    resolve(false);
                };
                elem.children[2].onclick=function(event) {
                    elem.remove();
                    elemBG.remove();
                    resolve(elem.children[1].value);
                };
            });
        }
        return false;
    }
}