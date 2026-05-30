/*class HalHash {
    constructor() {this.version="0.0.1";}
    addition(num) {
        if (num.length<2){return BigInteger.BigInt(num,10);}
        var h=parseInt(num.length/2);
        return BigInteger.add(BigInteger.add(BigInteger.BigInt(num.substring(0,h),10),BigInteger.BigInt(num.substring(h),10)),BigInteger.BigInt("1",10));
    }
    
    Str2Hash(str,length) {
        var out=[];
        if (str.length==0){out=[-1];}
        for (var i=0;i<str.length;i++){out.push(str.charCodeAt(i));}
        return this.Bin2Hash(new Uint8Array(out),length);
    }
    
    Bin2Hex(buffer) {
        return [...buffer]
        .map(x => x.toString(16).padStart(2, '0'))
        .join('');
    }

    Bin2Hash(bytes,length) {
        if (length<4){console.error("Minimum length=4")}
        var o=BigInteger.BigInt("0x"+this.Bin2Hex(bytes),16);
        var isNeg=o<0;
        o=o.toString(10);
        if (isNeg){o=o.substring(1);}
        if(o.length<4){
            var h=parseInt(o)/3+2.0;
            var j=4-o.length;
            while (parseInt(h).toString().length!=j) {
                while (parseInt(h).toString().length<j) {h=Math.pow(h,3);}
                while (parseInt(h).toString().length>j) {h/=3;}
            }
            o+=parseInt(h).toString();
        }
        var p=parseInt(o.length/4);
        var a=BigInteger.BigInt(o.substring(0,p),10);
        if (isNeg){a=BigInteger.sub(BigInteger.unaryMinus(a),BigInteger.BigInt("2"));}
        else {a=BigInteger.add(a,BigInteger.BigInt("2"));}
        var ad=a.toString(10).length;
        var b=BigInteger.BigInt(o.substring(p,p*2),10);
        if (isNeg){b=BigInteger.sub(BigInteger.unaryMinus(b),BigInteger.BigInt("2"));}
        else {b=BigInteger.add(b,BigInteger.BigInt("2"));}
        var bd=b.toString(10).length;
        var c=BigInteger.BigInt(o.substring(p*2,p*3),10);
        if (isNeg){c=BigInteger.sub(BigInteger.unaryMinus(c),BigInteger.BigInt("2"));}
        else {c=BigInteger.add(c,BigInteger.BigInt("2",10));}
        var cd=c.toString(10).length;
        var d=BigInteger.BigInt(o.substring(p*3),10);
        if (isNeg){d=BigInteger.sub(BigInteger.unaryMinus(d),BigInteger.BigInt("2"));}
        else {d=BigInteger.add(d,BigInteger.BigInt("2",10));}
        var dd=d.toString(10).length;
        var pd=parseInt(length/4);
        var od=length-pd*3;
        while (!(a.toString(16).length==pd && b.toString(16).length==pd && c.toString(16).length==pd && d.toString(16).length==od)) {
            while (a.toString(16).length<pd){a=BigInteger.exponentiate(a,BigInteger.BigInt("3",10));}
            while (a.toString(16).length>pd){a=this.addition(a.toString(10));}
            while (b.toString(16).length<pd){b=BigInteger.exponentiate(b,BigInteger.BigInt("3",10));}
            while (b.toString(16).length>pd){b=this.addition(b.toString(10));}
            while (c.toString(16).length<pd){c=BigInteger.exponentiate(c,BigInteger.BigInt("3",10));}
            while (c.toString(16).length>pd){c=this.addition(c.toString(10));}
            while (d.toString(16).length<od){d=BigInteger.exponentiate(d,BigInteger.BigInt("3",10));}
            while (d.toString(16).length>od){d=this.addition(d.toString(10));}
        }
        return a.toString(16)+b.toString(16)+c.toString(16)+d.toString(16);
    }
}*/

/*class HalHash {
    constructor() {this.version="0.0.2";}
    
    
    Str2Hash(str,length,count=64) {
        //var out=[];
        //if (str.length==0){out=[-1];}
        //for (var i=0;i<str.length;i++){out.push(str.charCodeAt(i));}
        //return this.Bin2Hash(new Uint8Array(out),length,count);
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
        return temp==""?BigInteger.BigInt(0):BigInteger.BigInt(temp);
    }

    Bin2Hash(bytes,length,count=64, maxIteration=100000) {
        //if (bytes.length==1){bytes=new Uint8Array([bytes[0],0]);}
        count=length<count?length:count;
        count=bytes.length<count?bytes.length:count;
        var sizeD=parseInt(bytes.length/count);
        var d=[];
        var i,b;
        var g=BigInteger.multiply(BigInteger.BigInt(count),BigInteger.BigInt(length));
        for (i=0;i<count;i++) {
            d.push(BigInteger.multiply(BigInteger.BigInt("0x"+this.Bin2Hex(bytes.slice(i*sizeD,i*sizeD+sizeD)),16),g));
        }
        if (bytes.length%count) {
            d[count-1]=BigInteger.multiply(BigInteger.BigInt("0x"+this.Bin2Hex(bytes.slice(count*sizeD-sizeD,bytes.length)),16),g);
        }
        
        var p=parseInt((length-count)/2);
        for (i=0;i<p;i++) {
            d.push(i+2);
            for (b=0;b<count;b++) {
                d[d.length-1]=BigInteger.add(BigInteger.multiply(d[b],d[d.length-1]),BigInteger.multiply(g,i+1));
            }
        }
        count=d.length;
        var temp=d.slice();
        
        g=BigInteger.add(this.getHalfInt(g,0),this.getHalfInt(g,1));
        for (i=0;i<count;i++) {
            for (b=0;b<count;b++) {
                //d[b]=BigInteger.multiply(d[b],BigInteger.add(BigInteger.add(d[b],BigInteger.multiply(temp[i],temp[i])),BigInteger.multiply(temp[i],g)));
                d[b]=BigInteger.add(d[b],BigInteger.add(BigInteger.multiply(temp[i],temp[i]),BigInteger.multiply(temp[i],g)));
            }
        }
        var h="",lc=count-1;
        var mc=0;
        while (1) {
            h=this.array_toHex(d);
            if (h.length<length) {
                for (i=0;i<count;i++) {
                    if (i==lc) {
                        //d[i]=BigInteger.add(BigInteger.multiply(this.getHalfInt(d[i],0),this.getHalfInt(d[i],1)),g);
                        d[i]=BigInteger.add(BigInteger.add(d[i],d[i]),g)
                    } else {
                        temp=BigInteger.add(this.getHalfInt(d[i],0),this.getHalfInt(d[i],1));
                        d[i]=BigInteger.BigInt(temp.toString()+(BigInteger.add(this.getHalfInt(d[i+1],0),this.getHalfInt(d[i+1],1))).toString());
                        d[i+1]=BigInteger.BigInt((BigInteger.add(this.getHalfInt(d[i+1],0),this.getHalfInt(d[i+1],1))).toString()+temp.toString());
                    }
                }
            } else if (h.length>length) {
                if (mc>maxIteration) {
                    h=h.substring(0,length);
                    break;
                }
                
                for (i=0;i<count;i++) {
                    if (i==lc) {
                        d[i]=BigInteger.add(BigInteger.add(this.getHalfInt(d[i],0),this.getHalfInt(d[i],1)),g);
                    } else {
                        temp=this.getHalfInt(d[i],1);
                        d[i]=BigInteger.add(this.getHalfInt(d[i],0),this.getHalfInt(d[i+1],1));
                        d[i+1]=BigInteger.add(this.getHalfInt(d[i+1],0),temp);
                    }
                }
            } else {break;}
            mc++;
        }
        return h;
    }
}*/

/*class HalHash {
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
}*/

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