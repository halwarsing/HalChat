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