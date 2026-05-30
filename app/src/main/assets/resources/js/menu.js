function onResizeMenu() {
    var bcr=document.getElementById("menu").getBoundingClientRect();
    document.getElementById("block-menu").style.height=(bcr.height+bcr.y)+"px";
}

function menuOpen() {
    if(document.getElementById(`pullMenuUser`).classList.contains(`pullMenuUserClosed`)){
        document.getElementById(`pullMenuUser`).classList.remove(`pullMenuUserClosed`);
        document.getElementById(`pullMenuUser`).classList.add(`pullMenuUserOpened`);
        document.getElementById(`menuUserArrow`).classList.remove(`menuUserArrowClosed`);
        document.getElementById(`menuUserArrow`).classList.add(`menuUserArrowOpened`);
    } else {
        document.getElementById(`pullMenuUser`).classList.remove(`pullMenuUserOpened`);
        document.getElementById(`pullMenuUser`).classList.add(`pullMenuUserClosed`);
        document.getElementById(`menuUserArrow`).classList.remove(`menuUserArrowOpened`);
        document.getElementById(`menuUserArrow`).classList.add(`menuUserArrowClosed`);
    }
}

function activateYandexMetrika() {
    (function(m,e,t,r,i,k,a){m[i]=m[i]||function(){(m[i].a=m[i].a||[]).push(arguments)};
    m[i].l=1*new Date();
    for (var j = 0; j < document.scripts.length; j++) {if (document.scripts[j].src === r) { return; }}
    k=e.createElement(t),a=e.getElementsByTagName(t)[0],k.async=1,k.src=r,a.parentNode.insertBefore(k,a)})
    (window, document, "script", "https://mc.yandex.ru/metrika/tag.js", "ym");

    ym(92574562, "init", {
        clickmap:true,
        trackLinks:true,
        accurateTrackBounce:true
    });
}

function notificationOpen() {
    if (document.getElementById(`pullNotification`).classList.contains(`pullNotificationClosed`)) {
        document.getElementById(`pullNotification`).classList.remove(`pullNotificationClosed`);
        document.getElementById(`pullNotification`).classList.add(`pullNotificationOpened`);
        //fetch("https://halwarsing.net/api/api?req=checkNotifications",{method:"GET",mode:"cors",credentials:"include"});
    } else {
        document.getElementById(`pullNotification`).classList.remove(`pullNotificationOpened`);
        document.getElementById(`pullNotification`).classList.add(`pullNotificationClosed`);
    }
}

let new_notification_audio=new Audio("file:///android_asset/resources/audio/new_notification.wav");

function clickNotificationBtn(e) {
    document.location.href=this.dataset.url;
}

function checkNotifications() {
    fetch("https://halwarsing.net/api/api?req=getNotifications",{method:"GET",mode:"cors",credentials:"include"})
    .then((response)=>response.json())
    .then((data)=> {
        if (data["errorCode"]===0) {
            if (data["isNewNotifications"]) {
                new_notification_audio.play().catch(e=>{});
            }
            if (data["notifications"].length>0) {
                document.getElementById("notificationNumber").textContent=data["count"];
                var isNew=false;
                for (var i=0;i<data["notifications"].length;i++) {
                    if (document.getElementById("notification"+data["notifications"][i]["uid"])===null) {
                        isNew=true;
                        break;
                    }
                }
                isNew=isNew||data["notifications"].length!=document.querySelectorAll(".notification").length;
                if (isNew) {
                    document.getElementById("pullNotification").innerHTML=`<div id="allNotificationsDiv"><a id="allNotifications" href="https://halwarsing.net/notifications">Все уведомления</a><div id="notificationSection"></div></div>`;
                    for (var i=data["notifications"].length-1;i>=0;i--) {
                        var elem=document.createElement("div");
                        elem.id="notification"+data["notifications"][i]["uid"];
                        elem.classList.add("notification");
                        var type=data["notifications"][i]["type"];
                        var jdata=JSON.parse(data["notifications"][i]["data"]);
                        if (type=="text") {
                            var textElem=document.createElement("span");
                            textElem.id="notificationText";
                            textElem.textContent=jdata["text"];
                            elem.appendChild(textElem);
                        } else if (type=="url") {
                            var urlElem=document.createElement("a");
                            urlElem.id="notificationUrl";
                            urlElem.textContent=jdata["text"];
                            urlElem.href=jdata["url"];
                            elem.appendChild(urlElem);
                        } else if (type=="buttons") {
                            var textElem=document.createElement("span");
                            textElem.id="notificationText";
                            textElem.textContent=jdata["text"];
                            elem.appendChild(textElem);
                            var divBtns=document.createElement("div");
                            divBtns.id="notificationDivBtns";
                            for (var j=0;j<jdata["buttons"].length;j++) {
                                var btn=document.createElement("input");
                                btn.id="notificationBtn";
                                btn.type="button";
                                btn.value=jdata["buttons"][j]["text"];
                                btn.dataset.url=jdata["buttons"][j]["url"];
                                btn.dataset.url=btn.dataset.url+(btn.dataset.url.indexOf("?")>-1?"&":"?")+"successUrl='."https://".$_SERVER["HTTP_HOST"].$_SERVER["REQUEST_URI"].'";
                                btn.addEventListener("click",clickNotificationBtn);
                                divBtns.appendChild(btn);
                            }
                            elem.appendChild(divBtns);
                        }
                        document.getElementById("pullNotification").appendChild(elem);
                        if (i>0){
                            var section=document.createElement("div");
                            section.id="notificationSection";
                            document.getElementById("pullNotification").appendChild(section);
                        }
                    }
                    document.getElementById("notificationNumber").textContent=data["count"];
                }
            } else if (document.getElementById("pullNotificationNothing")===null) {
                document.getElementById("pullNotification").innerHTML=`<span id="pullNotificationNothing">У вас нет уведомлений</span>`;
            }
        } else if (data["errorCode"]===1&&data["error"]==="Auth Error") {
            document.location.reload();
        } else if (data["errorCode"]===-1) {
            document.location.href="https://halwarsing.net/blockUserInfo";
        }
        setTimeout(checkNotifications,5000);
    }).catch((err)=>{console.log(err);setTimeout(checkNotifications,5000);});
}
checkNotifications();
window.addEventListener("resize",onResizeMenu);
onResizeMenu();
document.body.addEventListener("mousedown",function(e) {
    if (document.getElementById(`pullMenuUser`)!==null&&document.getElementById(`menuUser`)!==null&&document.getElementById(`pullMenuUser`).classList.contains(`pullMenuUserOpened`)&&e.target!==document.getElementById("menuUser")&&e.target.parentNode!==document.getElementById("menuUser")&&e.target.parentNode!==document.getElementById("pullMenuUser")&&e.target.parentNode.parentNode!==document.getElementById("pullMenuUser")) {
        document.getElementById(`pullMenuUser`).classList.remove(`pullMenuUserOpened`);
        document.getElementById(`pullMenuUser`).classList.add(`pullMenuUserClosed`);
        document.getElementById(`menuUserArrow`).classList.remove(`menuUserArrowOpened`);
        document.getElementById(`menuUserArrow`).classList.add(`menuUserArrowClosed`);
    }
    if (document.getElementById("pullNotification")!==null&&document.getElementById("notification")!==null&&document.getElementById("pullNotification").classList.contains("pullNotificationOpened")&&e.target!==document.getElementById("notification")&&e.target.parentNode!==document.getElementById("notification")&&e.target.parentNode!==document.getElementById("pullNotification")&&e.target.parentNode.parentNode!==document.getElementById("pullNotification")&&e.target.id!=="notificationBtn") {
        document.getElementById(`pullNotification`).classList.remove(`pullNotificationOpened`);
        document.getElementById(`pullNotification`).classList.add(`pullNotificationClosed`);
    }
    return true;
});
window.idUser=HCA.getUserId();
window.nicknameUser=HCA.getUserNickname();
window.iconUser=HCA.getUserIcon();
console.log(iconUser);

document.addEventListener("DOMContentLoaded",function(e) {
    document.getElementById("menuUserIcon").src=window.iconUser;
});