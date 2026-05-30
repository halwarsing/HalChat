class HalHash {
    constructor() {this.version="0.0.2";}
    
    
    Str2Hash(str,length,count=64) {
        return this.Bin2Hash((new TextEncoder("utf-8")).encode(str),length,count);
    }
    
    Bin2Hex(buffer) {
        return [...buffer]
        .map(x => x.toString(16).padStart(2, '0'))
        .join('');
    }
    
    array_toHex(arr) {
        var out="";
        for (var i=0;i<arr.length;i++) {
            out+=arr[i].toString(16);
        }
        return out;
    }
    
    getHalfInt(i,h) {
        var temp=i.toString();
        temp=h==0?temp.slice(0,parseInt(temp.length/2)):temp.slice(parseInt(temp.length/2),temp.length);
        return temp==""?BigInt(0):BigInt(temp);
    }

    Bin2Hash(bytes,length,count=64, maxIteration=100000) {
        //if (bytes.length==1){bytes=new Uint8Array([bytes[0],0]);}
        count=length<count?length:count;
        count=bytes.length<count?bytes.length:count;
        var sizeD=parseInt(bytes.length/count);
        var d=[];
        var i,b;
        var g=BigInt(count)*BigInt(length);
        for (i=0;i<count;i++) {
            d.push(BigInt("0x"+this.Bin2Hex(bytes.slice(i*sizeD,i*sizeD+sizeD)),16)*g);
        }
        if (bytes.length%count) {
            d[count-1]=BigInt("0x"+this.Bin2Hex(bytes.slice(count*sizeD-sizeD,bytes.length)),16)*g;
        }
        
        var p=parseInt((length-count)/2);
        for (i=0;i<p;i++) {
            d.push(BigInt(i+2));
            for (b=0;b<count;b++) {
                d[d.length-1]=d[b]*d[d.length-1]+g*BigInt(i+1);
            }
        }
        count=d.length;
        var temp=d.slice();
        
        g=this.getHalfInt(g,0)+this.getHalfInt(g,1);
        for (i=0;i<count;i++) {
            for (b=0;b<count;b++) {
                d[b]=d[b]+temp[i]*temp[i]+temp[i]*g;
            }
        }
        var h="",lc=count-1;
        var mc=0;
        while (1) {
            h=this.array_toHex(d);
            if (h.length<length) {
                for (i=0;i<count;i++) {
                    if (i==lc) {
                        d[i]=d[i]+d[i]+g;
                    } else {
                        temp=this.getHalfInt(d[i],0)+this.getHalfInt(d[i],1);
                        d[i]=BigInt(temp.toString()+(this.getHalfInt(d[i+1],0)+this.getHalfInt(d[i+1],1)).toString());
                        d[i+1]=BigInt((this.getHalfInt(d[i+1],0)+this.getHalfInt(d[i+1],1)).toString()+temp.toString());
                    }
                }
            } else if (h.length>length) {
                if (mc>maxIteration) {
                    h=h.substring(0,length);
                    break;
                }
                
                for (i=0;i<count;i++) {
                    if (i==lc) {
                        d[i]=this.getHalfInt(d[i],0)+this.getHalfInt(d[i],1)+g;
                    } else {
                        temp=this.getHalfInt(d[i],1);
                        d[i]=this.getHalfInt(d[i],0)+this.getHalfInt(d[i+1],1);
                        d[i+1]=this.getHalfInt(d[i+1],0)+temp;
                    }
                }
            } else {break;}
            mc++;
        }
        return h;
    }
}

/*class HalHash {
    constructor() {this.version="0.0.3";}
    
    
    Str2Hash(str,length,count=64) {
        return this.Bin2Hash((new TextEncoder("utf-8")).encode(str),length,count);
    }
    
    Bin2Hex(buffer) {
        // Используем Array.from вместо spread для лучшей производительности
        return Array.from(buffer).map(x => x.toString(16).padStart(2, '0')).join('');
    }
    
    array_toHex(arr) {
        // Используем reduce для более лаконичного и эффективного кода
        return arr.reduce((out, value) => out + value.toString(16), "");
    }
    
    getHalfInt(i, h) {
        const temp = i.toString();
        return BigInt(h === 0 ? temp.slice(0, Math.floor(temp.length / 2)) : temp.slice(Math.floor(temp.length / 2), temp.length));
    }

    Bin2Hash(bytes,length,count=64, maxIteration=100000) {
        //if (bytes.length==1){bytes=new Uint8Array([bytes[0],0]);}
        count = Math.min(length, count, bytes.length);
        //var sizeD=parseInt(bytes.length/count);
        const sizeD = Math.floor(bytes.length / count);
        let d = [];
        let g = BigInt(count) * BigInt(length);
        for (let i = 0; i < count; i++) {
            // Избегаем лишних преобразований в строку и обратно
            d.push(BigInt("0x"+this.Bin2Hex(bytes.slice(i * sizeD, i * sizeD + sizeD)), 16) * g);
        }
        if (bytes.length % count) {
            d[count - 1] = BigInt("0x"+this.Bin2Hex(bytes.slice(count * sizeD - sizeD, bytes.length)), 16) * g;
        }
        
        const p = Math.floor((length - count) / 2);
        for (let i = 0; i < p; i++) {
            d.push(BigInt(i + 2));
            for (let b = 0; b < count; b++) {
                d[d.length - 1] = d[b] * d[d.length - 1] + g * BigInt(i + 1);
            }
        }
        count=d.length;
        let temp = d.slice();
        g = this.getHalfInt(g, 0) + this.getHalfInt(g, 1);
        for (let i = 0; i < count; i++) {
            for (let b = 0; b < count; b++) {
                d[b] = d[b] + temp[i] * temp[i] + temp[i] * g;
            }
        }
        let h = "", lc = count - 1, mc = 0;
        while (true) {
            h = this.array_toHex(d);
            if (h.length < length) {
                for (let i = 0; i < count; i++) {
                    if (i === lc) {
                        d[i] = d[i] + d[i] + g;
                    } else {
                        let temp = this.getHalfInt(d[i], 0) + this.getHalfInt(d[i], 1);
                        d[i] = BigInt(temp.toString() + (this.getHalfInt(d[i + 1], 0) + this.getHalfInt(d[i + 1], 1)).toString());
                        d[i + 1] = BigInt((this.getHalfInt(d[i + 1], 0) + this.getHalfInt(d[i + 1], 1)).toString() + temp.toString());
                    }
                }
            } else if (h.length > length) {
                if (mc > maxIteration) {
                    h = h.substring(0, length);
                    break;
                }

                for (let i = 0; i < count; i++) {
                    if (i === lc) {
                        d[i] = this.getHalfInt(d[i], 0) + this.getHalfInt(d[i], 1) + g;
                    } else {
                        let temp = this.getHalfInt(d[i], 1);
                        d[i] = this.getHalfInt(d[i], 0) + this.getHalfInt(d[i + 1], 1);
                        d[i + 1] = this.getHalfInt(d[i + 1], 0) + temp;
                    }
                }
            } else {
                break;
            }
            mc++;
        }
        return h;
    }
}*/

class HalEncryption {
    constructor() {this.version="0.0.3";this.hh=new HalHash();}
    
    shift_forward(a,b) {
        var o=a+b;
        return (o>255)?o-256:o;
    }
    
    shift_back(a,b) {
        var o=a-b;
        return (o<0)?o+256:o;
    }
    
    encode(data,passw,hashLength=256,hashCount=64,secretData="") {return this.encodeByHash(data,this.hh.Str2Hash(passw,hashLength,hashCount),hashCount,secretData);}
    
    encodeByHash(data,passw,hashCount=64,secretData="") {
        var out=[];
        var a=0;
        var b=passw.length-1;
        var newPassw=this.hh.Str2Hash(passw+passw+secretData,passw.length,hashCount);
        for (var i=0;i<data.length;i++) { 
            out.push(this.shift_forward(data[i],parseInt(newPassw.substring(a,a+2),16)));
            a++;
            if (a>=b){newPassw=this.hh.Str2Hash(passw+newPassw,passw.length,hashCount);a=0;}
        }
        return new Uint8Array(out);
    }
    
    decode(data,passw,hashLength=256,hashCount=64,secretData="") {return this.decodeByHash(data,this.hh.Str2Hash(passw,hashLength,hashCount),hashCount,secretData);}
    
    decodeByHash(data,passw,hashCount=64,secretData="") {
        var out=[];
        var a=0;
        var b=passw.length-1;
        var newPassw=this.hh.Str2Hash(passw+passw+secretData,passw.length,hashCount);
        for (var i=0;i<data.length;i++) {
            out.push(this.shift_back(data[i],parseInt(newPassw.substring(a,a+2),16)));
            a++;
            if (a>=b){newPassw=this.hh.Str2Hash(passw+newPassw,passw.length,hashCount);a=0;}
        }
        return new Uint8Array(out);
    }
}

var he=new HalEncryption();

onmessage=function(e) {
    var data=JSON.parse(e.data);
    var msgs=data[0];
    var chatUid=data[1];
    var isBot=data[2];
    var password=data[3];
    var messageB,answerMsgText,i;
    var out=[];
    for (var msg of msgs) {
        messageB=[];
        for (i=0;i<msg['message'].length;i+=2) {
            messageB.push(parseInt(msg['message'].slice(i,i+2),16));
        }
        messageB=new Uint8Array(messageB);
        if (msg['answerMsg']!=-1&&msg['answerMsgTextEncoded']==true&&!isBot) {
            answerMsgText=[];
            for (i=0;i<msg['answerMsgText'].length;i+=2) {
                answerMsgText.push(parseInt(msg['answerMsgText'].slice(i,i+2),16));
            }
            answerMsgText=new Uint8Array(answerMsgText);
            msg['answerMsgText']=new TextDecoder().decode(he.decodeByHash(answerMsgText,password+msg['answerMsgEncryptId'],10));
        }
        msg['message']=isBot?msg['message']:new TextDecoder().decode(he.decodeByHash(messageB,password+msg['encryptId'],10));
        out.push(msg);
    }
    postMessage(out);
}