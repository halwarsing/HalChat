let titles_seconds=["секунду","секунды","секунд"],titles_minutes=["минуту","минуты","минут"],titles_hours=["час","часа","часов"],titles_days=["день","дня","дней"],titles_weeks=["неделю","недели","недель"],titles_months=["месяц","месяца","месяцев"];
let hdw=new HalDialogWindow("#414141","#FFFFFF","#FFFFFF","#000000");
let addNewChatDiv=document.getElementById("addNewChatDiv");
let chatList=document.getElementById("chatList");
let chats;
let he=new HalEncryption();

function startTraining() {
    document.location.href="https://halchat.halwarsing.net/training?successUrl=<?php echo 'https://'.$_SERVER['HTTP_HOST'].$_SERVER['REQUEST_URI'];?>";
}

function cancelTraining() {
    document.getElementById("training").remove();
    fetch('https://halchat.halwarsing.net/api?req=trainingComplete');
}

function case_num(number,titles,cases) {
    return titles[(number%100>4&&number%100<20)?2:cases[(number%10<5)?number%10:5]];
}

function getTimeFromSeconds(seconds) {
    if (seconds<60) {
        return seconds+" "+case_num(seconds,titles_seconds,[2, 0, 1, 1, 1, 2])+" назад";
    } else if (seconds<3600) {
        var minutes=parseInt(seconds/60);
        return minutes+" "+case_num(minutes,titles_minutes,[2,0,1,1,1,2])+" назад"
    } else if (seconds<86400) {
        var hours=parseInt(seconds/3600);
        return hours+" "+case_num(hours,titles_hours,[2,0,1,1,1,2])+" назад";
    } else if (seconds<604800) {
        var days=parseInt(seconds/86400);
        return days+" "+case_num(days,titles_days,[2,0,1,1,1,2])+" назад";
    } else if (seconds<2592000) {
        var weeks=parseInt(seconds/604800);
        return weeks+" "+case_num(weeks,titles_weeks,[2,0,1,1,1,2])+" назад";
    } else if (seconds<31536000) {
        var months=parseInt(seconds/2592000);
        return months+" "+case_num(months,titles_months,[2,0,1,1,1,2])+" назад";
    } else {
        var years=parseInt(seconds/31536000);
        return years+" "+(years==1?"год":(years<5?"года":"лет"))+" назад";
    }
}

function checkExistsUID(uid,chats) {
    for (var i=0;i<chats.length;i++) {
        if (uid==chats[i]['uid']) {return i;}
    }
    return null;
}

function byteToHex(b) {
    return ("0" + b.toString(16)).substr(-2);
}

function checkNewChats() {
    fetch('https://halchat.halwarsing.net/api?req=getListChats',{method:'GET',mode:'cors',credentials:'include'}).then((response)=>response.json()).then((data)=>{
        if (data['errorCode']===0) {
            var elems=document.querySelectorAll("#chat");
            for (var i=0;i<elems.length;i++) {
                var elem=elems[i];
                var chati=checkExistsUID(elem.dataset.uid,data['chats']);
                if (chati!==null) {
                    if (elem.dataset.isExistsPassword==="true") {
                        if (data['chats'][chati]['lastMessage']===null) {
                            if (elem.children[1].children[1].textContent!=="Создан новый чат") {
                                elem.children[1].children[1].textContent="Создан новый чат";
                                var now=new Date();
                                var curDate = Date.UTC(now.getUTCFullYear(),now.getUTCMonth(),now.getUTCDate(),now.getUTCHours(), now.getUTCMinutes(), now.getUTCSeconds(), now.getUTCMilliseconds());
                                elem.children[2].textContent=getTimeFromSeconds(parseInt((curDate-chatLastMsgDate)/1000));
                                if (chatList.children.length>1&&chatList.children[0]!==elem) {
                                    chatList.insertBefore(elem,addNewChatDiv);
                                }
                            }
                        } else {
                            if (elem.children[1].children[1].textContent==="Создан новый чат"||data['chats'][chati]['lastMessage']['time']!=elem.dataset.time) {
                                var chatLastMsgDate=new Date(data['chats'][chati]['lastMessage']['time']*1000);
                                var message=[];
                                if (data['chats'][chati]['lastMessage']['attachments'].length===0) {
                                    for (j=0;j<data['chats'][chati]['lastMessage']['message'].length;j+=2) {
                                        message.push(parseInt(data['chats'][chati]['lastMessage']['message'].slice(j,j+2),16));
                                    }
                                    var msg=new TextDecoder().decode(he.decodeByHash(message,localStorage.getItem(data['chats'][chati]['uid'])+data['chats'][chati]['lastMessage']['encryptId'],10));
                                    var pattern_emoji=/\[emoji-(.*?)\]/gim;
                                    msg=msg.replace(pattern_emoji,'<img src="/resources/emoji/$1.png" id="emoji">');
                                    elem.children[1].children[1].innerHTML=data['chats'][chati]['lastMessage']['fromNickname']+": "+msg;
                                } else {
                                    elem.children[1].children[1].textContent=data['chats'][chati]['lastMessage']['fromNickname']+": "+(data['chats'][chati]['lastMessage']['attachments'].length===1?"Прикреплён файл":"Прикреплены файлы");
                                }
                                var now=new Date();
                                var curDate = Date.UTC(now.getUTCFullYear(),now.getUTCMonth(),now.getUTCDate(),now.getUTCHours(), now.getUTCMinutes(), now.getUTCSeconds(), now.getUTCMilliseconds());
                                elem.children[2].textContent=getTimeFromSeconds(parseInt((curDate-chatLastMsgDate)/1000));
                                if (chatList.children.length>1&&chatList.children[0]!==elem) {
                                    chatList.insertBefore(elem,addNewChatDiv);
                                }
                            }
                        }
                    }
                } else {elem.remove();}
            }
        }
    });
    setTimeout(checkNewChats,5000);
}

function selectChat(e) {
    if (isSelectMode) {
        if (this.children[0].dataset.selected==="false") {
            this.children[0].dataset.selected="true";
        } else {
            this.children[0].dataset.selected="false";
        }
    } else {
        document.location.href="https://halchat.halwarsing.net/chat/"+this.dataset.id;
    }
}

function sendMessages(text) {
    var elems=document.querySelectorAll('#chatIconDiv[data-selected="true"]');
    if (elems.length===0) {
        hdw.alert('Вы не выбрали чаты');
    } else {
        hdw.prompt('Напишите текст','Отправить',text).then(async(d)=>{
            if (d) {
                isSelectMode=false;
                document.getElementById("selectChatsBtn").remove();
                var elem;
                for (var i=0;i<elems.length;i++) {
                    elems[i].dataset.selected="false";
                    elem=elems[i].parentNode;
                    if (elem.dataset.isExistsPassword==="true") {
                        var formData = new FormData();
                        var encryptId=hh.Str2Hash(Date.now()+":"+elem.dataset.id+":"+text.length,16,16);
                        formData.append('message',[...he.encodeByHash(new TextEncoder("utf-8").encode(text),localStorage.getItem(elem.dataset.uid)+encryptId,10)].map(x=>byteToHex(x)).join(""));
                        formData.append('attachments','[]');
                        fromData.append('encryptId',encryptId);
                        await fetch("https://halchat.halwarsing.net/api?req=sendMessage&chatId="+elem.dataset.uid,{
                            method:"POST",
                            mode:"cors",
                            credentials:"include",
                            body:formData
                        });
                        if (chatList.children.length>1&&chatList.children[0]!==elem) {
                            chatList.insertBefore(elem,addNewChatDiv);
                        }
                    }
                }
            }
        });
    }
}

function startHalwarsingNetChat() {
    fetch('https://halchat.halwarsing.net/api?req=getListChats',{method:'GET',mode:'cors',credentials:'include'}).then((response)=>response.json()).then((data)=>{
        if (data['errorCode']===0) {
            if (data['chats'].length===0) {
                chatList.innerHTML=`
                <div id="addNewChatDiv">
                    <img src="https://halchat.halwarsing.net/resources/icons/create-chat.png" id="addNewChatImg" draggable="false">
                    <a href="https://halchat.halwarsing.net/createChat" id="addNewChatSpan">Создать чат</a>
                </div>`;
            } else {
                /*chatList.style.borderTop="1px solid #707070";
                chatList.style.borderLeft="1px solid #707070";
                chatList.style.borderRight="1px solid #707070";
                chatList.style.borderBottom="1px solid #707070";*/
                var message,j;
                data['chats']=data['chats'].sort((a,b)=>{
                    var atime,btime;
                    if (a['lastMessage']===null||localStorage.getItem(a['uid'])===null) {
                        atime=a['created'];
                    } else {
                        atime=a['lastMessage']['time'];
                    }
                    if (b['lastMessage']===null||localStorage.getItem(b['uid'])===null) {
                        btime=b['created'];
                    } else {
                        btime=b['lastMessage']['time'];
                    }
                    return atime<btime?1:-1;
                });
                for (var i=0;i<data['chats'].length;i++) {
                    const chat=data['chats'][i];
                    var elemChat=document.createElement("div");
                    elemChat.id="chat";
                    elemChat.dataset.uid=chat['uid'];
                    elemChat.dataset.isExistsPassword=localStorage.getItem(chat['uid'])===null?'false':'true';
                    
                    var chatIconDiv=document.createElement("div");
                    chatIconDiv.id="chatIconDiv";
                    chatIconDiv.dataset.selected="false";
                    var chatIcon=document.createElement("img");
                    chatIcon.id="chatIcon";
                    chatIcon.src="https://haldrive.halwarsing.net/file/"+chat['icon'];
                    chatIconDiv.appendChild(chatIcon);
                    elemChat.appendChild(chatIconDiv);
                    
                    var chatNameLastMsgDiv=document.createElement("div");
                    chatNameLastMsgDiv.id="chatNameLastMsgDiv";
                    
                    var chatName=document.createElement("span");
                    chatName.id="chatName";
                    chatName.textContent=chat['name'];
                    chatNameLastMsgDiv.appendChild(chatName);
                    
                    var chatLastMsg=document.createElement("div");
                    chatLastMsg.id="chatLastMsg";
                    
                    var chatLastMsgDate;
                    
                    if (chat['lastMessage']===null) {
                        elemChat.dataset.time=chat['created'];
                        chatLastMsgDate=new Date(chat['created']*1000);
                        chatLastMsg.textContent="Создан новый чат";
                    } else if (localStorage.getItem(chat['uid'])===null) {
                        elemChat.dataset.time=chat['created'];
                        chatLastMsgDate=new Date(chat['created']*1000);
                        chatLastMsg.textContent="Введите пароль чтобы расшифровать";
                    } else {
                        elemChat.dataset.time=chat['lastMessage']['time'];
                        chatLastMsgDate=new Date(chat['lastMessage']['time']*1000);
                        message=[];
                        if (chat['lastMessage']['attachments'].length===0) {
                            for (j=0;j<chat['lastMessage']['message'].length;j+=2) {
                                message.push(parseInt(chat['lastMessage']['message'].slice(j,j+2),16));
                            }
                            var msg=new TextDecoder().decode(he.decodeByHash(message,localStorage.getItem(chat['uid'])+chat['lastMessage']['encryptId'],10));
                            var pattern_emoji=/\[emoji-(.*?)\]/gim;
                            msg=msg.replace(pattern_emoji,'<img src="/resources/emoji/$1.png" id="emoji">');
                            chatLastMsg.innerHTML=chat['lastMessage']['fromNickname']+": "+msg;
                        } else {
                            chatLastMsg.textContent=chat['lastMessage']['fromNickname']+": "+(chat['lastMessage']['attachments'].length===1?"Прикреплён файл":"Прикреплены файлы");
                        }
                    }
                    chatNameLastMsgDiv.appendChild(chatLastMsg)
                    
                    elemChat.appendChild(chatNameLastMsgDiv);
                    
                    var chatLastMsgTime=document.createElement("span");
                    var now=new Date();
                    var curDate = Date.UTC(now.getUTCFullYear(),now.getUTCMonth(),now.getUTCDate(),now.getUTCHours(), now.getUTCMinutes(), now.getUTCSeconds(), now.getUTCMilliseconds());
                    chatLastMsgTime.id="chatLastMsgTime";
                    chatLastMsgTime.textContent=getTimeFromSeconds(parseInt((curDate-chatLastMsgDate)/1000));
                    
                    elemChat.appendChild(chatLastMsgTime);
                    
                    elemChat.dataset.id=chat['id'];
                    elemChat.addEventListener("click",selectChat);
                    
                    chatList.appendChild(elemChat);
                }
            }
            setTimeout(checkNewChats,5000);
        }
    });
}

startHalwarsingNetChat();