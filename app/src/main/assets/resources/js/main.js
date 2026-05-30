let onReceiveMsg=null;
let openedChatId=-1;
let idUser=-1;

function getTask() {
    if (typeof(HalwarsingNetChat)!=='undefined') {
        /*fetch("/getTask",{method:"GET",mode:"cors",credentials:"include"})
        .then((response)=>response.json())
        .then((data)=>{
            if (data['errorCode']==0) {
                if (data['type']=='createChat') {
                    HalwarsingNetChat.createReceivedChat(parseInt(data['fromId']),data['data']);
                } else if (data['type']=='sendMsg') {
                    var dat=data['data'].split(";");
                    var msgd=dat.slice(4,dat.length).join(";");
                    var attc=JSON.parse(dat[3]);
                    HalwarsingNetChat.receiveMessage(parseInt(data['fromId']),msgd,attc,dat[2]=='1',dat[1]);
                    if (onReceiveMsg!=null&&openedChatId==parseInt(data['fromId'])) {
                        onReceiveMsg(false,msgd,dat[1],attc,dat[2]=='1');
                    }
                } else if (data['type']=='receiveMsg') {
                    HalwarsingNetChat.setMessageIsReceived(parseInt(data['fromId']),parseInt(data['data']));
                }
            }
            setTimeout(getTask,500);
        });*/
    } else {
        setTimeout(getTask,500);
    }
}

function checkHalwarsingNetChat() {
    /*if (typeof(HalwarsingNetChat)!=='undefined'&&idUser!=-1) {
        var elems=[];
        var pullMenuUserDivChats=document.createElement("div");
        pullMenuUserDivChats.id="pullMenuUserDiv";
        pullMenuUserDivChats.setAttribute("onclick","document.location.href='https://halchat.halwarsing.net/selectChat?isApp=1'");
        pullMenuUserDivChats.innerHTML=`<img src="https://halchat.halwarsing.net/resources/icons/chats.png" draggable="false"><a href="https://halchat.halwarsing.net/selectChat?isApp=1">Чаты</a>`;
        elems.push(pullMenuUserDivChats);
        
        var pullMenuUserSection=document.createElement("div");
        pullMenuUserSection.id="pullMenuUserSection";
        elems.push(pullMenuUserSection);
        
        var pullMenuUserDivCreateChat=document.createElement("div");
        pullMenuUserDivCreateChat.id="pullMenuUserDiv";
        pullMenuUserDivCreateChat.setAttribute("onclick","document.location.href='https://halchat.halwarsing.net/createChat?isApp=1'");
        pullMenuUserDivCreateChat.innerHTML=`<img src="https://halchat.halwarsing.net/resources/icons/create-chat.png" draggable="false"><a href="https://halchat.halwarsing.net/createChat?isApp=1">Создать чат</a>`;
        elems.push(pullMenuUserDivCreateChat);
        
        elems.push(pullMenuUserSection.cloneNode());
        
        var c=document.getElementById("pullMenuUser").children[0];
        for (var elem of elems) {
            document.getElementById("pullMenuUser").insertBefore(elem,c);
        }
        startHalwarsingNetChat();
        getTask();
    } else {
        setTimeout(checkHalwarsingNetChat,200);
    }*/
}

function loadMain() {
    var elems=[];
    var pullMenuUserDivChats=document.createElement("div");
    pullMenuUserDivChats.id="pullMenuUserDiv";
    pullMenuUserDivChats.setAttribute("onclick","document.location.href='https://halchat.halwarsing.net/selectChat'");
    pullMenuUserDivChats.innerHTML=`<img src="https://halchat.halwarsing.net/resources/icons/chats.png" draggable="false"><a href="https://halchat.halwarsing.net/selectChat">Чаты</a>`;
    elems.push(pullMenuUserDivChats);
    
    var pullMenuUserSection=document.createElement("div");
    pullMenuUserSection.id="pullMenuUserSection";
    elems.push(pullMenuUserSection);
    
    var pullMenuUserDivCreateChat=document.createElement("div");
    pullMenuUserDivCreateChat.id="pullMenuUserDiv";
    pullMenuUserDivCreateChat.setAttribute("onclick","document.location.href='https://halchat.halwarsing.net/createChat'");
    pullMenuUserDivCreateChat.innerHTML=`<img src="https://halchat.halwarsing.net/resources/icons/create-chat.png" draggable="false"><a href="https://halchat.halwarsing.net/createChat">Создать чат</a>`;
    elems.push(pullMenuUserDivCreateChat);
    
    elems.push(pullMenuUserSection.cloneNode());
    
    var c=document.getElementById("pullMenuUser").children[0];
    for (var elem of elems) {
        document.getElementById("pullMenuUser").insertBefore(elem,c);
    }
}