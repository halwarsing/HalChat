var Module={
    setStatus:null,
    preRun:[],
    postRun:[],
    print:function(text) {
        console.log(text);
    },
    printErr:function(){},
    printErrorJS:function(text) {
        console.error(text);
    },
    stdin:function(text) {
        return prompt(text);
    },
    is_load_module_resolve:false,
    is_load_module:false,
    canvas:null,
    HalSM:{
        wasmInstalled:false,
        initialized:false,
        HalSMVariableType: {},
        JavascriptVariables:{}
    },
    onLoadHSMEFile:function(index) {
        Module.ccall("run","number",["number"],[index],{async:true});
    },
    addContentMessage:function(){},
    htmlevents:{},
    openWindow:function(){},
    styleElem:document.createElement("style"),
    eventDatas:[],
    showMessage:function(){},
}

class HalChatPlugins {
    constructor() {
        this.events={onUserSendMessage:[],onUserAddMessage:[],onStart:[]};
    }
    
    jsValueToHalSMVar(arg) {
        if (typeof arg==='string'||arg instanceof String) {
            return Module.ccall("stringToVar","number",["string"],[arg]);
        } else if (typeof arg==='number'&&Number.isInteger(arg)) {
            return Module._variableInit(Module._intToValue(arg),Module.HalSM.HalSMVariableType.int);
        } else if (typeof arg==='number') {
            return Module._variableInit(Module.ccall("floatToValue","number",["string"],[arg.toString()]),Module.HalSM.HalSMVariableType.double);
        } else if (typeof arg==='boolean') {
            return arg===true?Module._getHalSMTrue():Module._getHalSMFalse();
        } else if (Array.isArray(arg)) {
            return Module._variableInit(this.getHalSMArguments(arg),Module.HalSM.HalSMVariableType.HalSMArray);
        } else if (arg===null) {
            return Module._getHalSMNull();
        } else if (arg.constructor === Object) {
            return Module._variableInit(this.dictToHalSMDict(arg),Module.HalSM.HalSMVariableType.HalSMDict);
        }
    }
    
    dictToHalSMDict(dict) {
        var i;
        var out=Module._initDict();
        var key;
        var keys=Object.keys(dict);
        var dictelem;
        for (i=0;i<keys.length;i++) {
            key=keys[i];
            dictelem=Module._initDictElement(Module.ccall("stringToVar","number",["string"],[key]),this.jsValueToHalSMVar(dict[key]));
            Module._putToDict(out,dictelem);
        }
        return out;
    }
    
    getHalSMArguments(args) {
        var out=Module._initHalSMArray();
        var i;
        for (i=0;i<args.length;i++) {
            const arg=args[i];
            Module._appendToHalSMArray(out,this.jsValueToHalSMVar(arg));
        }
        return out;
    }
    
    addEvent(name,func,namePluginData="") {
        if (Object.keys(this.events).indexOf(name)===-1) {
            return -1;
        }
        this.events[name].push([func,namePluginData]);
        return this.events[name].length-1;
    }
    
    runEvent(name,args,pluginTag={}) {
        if (Object.keys(this.events).indexOf(name)===-1) {
            return false;
        }
        var i,l;
        var a=this.events[name];
        l=a.length;
        var argsH=this.getHalSMArguments(args);
        var lindex=Module._getSizeHalSMArray(argsH);
        var argPlugin=Module._getHalSMNull();
        Module._appendToHalSMArray(argsH,argPlugin);
        for (i=0;i<l;i++) {
            if (a[i][1].length===0||a[i][1] in pluginTag) {
                Module._setToHalSMArray(argsH,this.jsValueToHalSMVar(a[i][1] in pluginTag?pluginTag[a[i][1]]:false),lindex);
                Module._runLocalFunction(a[i][0],argsH,Module.HalSM.nulldict);
            }
        }
        return true;
    }
    
    async loadPlugin(path) {
        Module.is_load_module=new Promise(function(resolve,reject){Module.is_load_module_resolve=resolve;});
        await Module.ccall("load","number",["string"],[path],{async:true});
        await Module.is_load_module;
    }
}

let MainHalChatPlugins=new HalChatPlugins();



class HalChatHTML {
    //Button
    //Input
    //Text
    //Block HTML
    
    static addButton(hsmc,args,vrs) {
        //(text,onclick)
        var lArgs=Module._getSizeHalSMArray(args);
        if (lArgs<3||lArgs>4) {return Module.HalSM.null;}
        var nameEventVar=Module._getVariableFromHalSMArray(args,1);
    }
    
    static getHTML(classc) {
        classc=Module._getHalSMClassCFromValue(Module._getValueVariable(classc));
        return CharArrayToString(Module._getStringFromValue(Module._getValueVariable(Module._getVariableFromClassC(classc,Module.ccall("stringToVar","number",["string"],["html"])))));
    }
}

class HalChatModule {
    static funcs={"sendMessage":HalChatModule.sendMessage,"addEvent":HalChatModule.addEvent,"addContentMessage":HalChatModule.addContentMessage,"makeHTMLEvent":HalChatModule.makeHTMLEvent,"openWindow":HalChatModule.openWindow,"addStyle":HalChatModule.addStyle,"eventData":HalChatModule.eventData,"showMessage":HalChatModule.showMessage};
    static clsses={};
    
    static sendMessage(hsmc,args,vrs) {
        var lArgs=Module._getSizeHalSMArray(args);
        if (lArgs!=7) {return Module.HalSM.null;}
        var textMessageVar=Module._getVariableFromHalSMArray(args,1);
        var attachmentsVar=Module._getVariableFromHalSMArray(args,2);
        var encryptIdVar=Module._getVariableFromHalSMArray(args,3);
        var soundMsgVar=Module._getVariableFromHalSMArray(args,4);
        var answerMsgVar=Module._getVariableFromHalSMArray(args,5);
        var commentMsgVar=Module._getVariableFromHalSMArray(args,6);
        if (Module._getTypeVariable(Module._getVariableFromHalSMArray(args,0))===Module.HalSM.HalSMVariableType.HalSMCModule&&Module._getTypeVariable(textMessageVar)===Module.HalSM.HalSMVariableType.str&&Module._getTypeVariable(attachmentsVar)===Module.HalSM.HalSMVariableType.HalSMArray&&
        Module._getTypeVariable(encryptIdVar)===Module.HalSM.HalSMVariableType.str&&Module._getTypeVariable(soundMsgVar)===Module.HalSM.HalSMVariableType.str&&Module._getTypeVariable(answerMsgVar)===Module.HalSM.HalSMVariableType.int&&Module._getTypeVariable(commentMsgVar)===Module.HalSM.HalSMVariableType.int) {
            var textMessage=CharArrayToString(Module._getStringFromValue(Module._getValueVariable(textMessageVar)));
            var attachmentsHalSMArray=Module._getValueVariable(attachmentsVar);
            var attachmentsArray=[];
            var encryptId=CharArrayToString(Module._getStringFromValue(Module._getValueVariable(encryptIdVar)));
            var soundMsg=CharArrayToString(Module._getStringFromValue(Module._getValueVariable(soundMsgVar)));
            var answerMsg=Module._getLongIntFromValue(Module._getValueVariable(answerMsgVar));
            var commentMsg=Module._getLongIntFromValue(Module._getValueVariable(commentMsgVar));
            var l=Module._getSizeHalSMArray(attachmentsHalSMArray);
            for (var i=0;i<l;i++) {
                var attachmentVar=Module._getVariableFromHalSMArray(attachmentsHalSMArray,i);
                if (Module._getTypeVariable(attachmentVar)!==Module.HalSM.HalSMVariableType.str) {
                    return Module.HalSM.false;
                }
                attachmentsArray.push(CharArrayToString(Module._getStringFromValue(Module._getValueVariable(attachmentVar))));
            }
            return Module.HalSM.true;
        }
        return Module.HalSM.false;
    }
    
    static initializeVars() {
        return {};
    }
    
    static addEvent(hsmc,args,vrs) {
        var lArgs=Module._getSizeHalSMArray(args);
        if (lArgs<3||lArgs>4) {return Module.HalSM.null;}
        var nameEventVar=Module._getVariableFromHalSMArray(args,1);
        var funcEventVar=Module._getVariableFromHalSMArray(args,2);
        if (Module._getTypeVariable(Module._getVariableFromHalSMArray(args,0))===Module.HalSM.HalSMVariableType.HalSMCModule&&Module._getTypeVariable(nameEventVar)===Module.HalSM.HalSMVariableType.str&&Module._getTypeVariable(funcEventVar)===Module.HalSM.HalSMVariableType.HalSMLocalFunction) {
            if (lArgs===4) {
                var namePluginDataVar=Module._getVariableFromHalSMArray(args,3);
                if (Module._getTypeVariable(namePluginDataVar)===Module.HalSM.HalSMVariableType.str) {
                    return Module._variableInit(Module._intToValue(MainHalChatPlugins.addEvent(CharArrayToString(Module._getStringFromValue(Module._getValueVariable(nameEventVar))),Module._getValueVariable(funcEventVar),CharArrayToString(Module._getStringFromValue(Module._getValueVariable(namePluginDataVar))))),Module.HalSM.HalSMVariableType.int);
                }
            }
            return Module._variableInit(Module._intToValue(MainHalChatPlugins.addEvent(CharArrayToString(Module._getStringFromValue(Module._getValueVariable(nameEventVar))),Module._getValueVariable(funcEventVar))),Module.HalSM.HalSMVariableType.int);
        }
        return Module.HalSM.null;
    }
    
    static addContentMessage(hsmc,args,vrs) {
        var lArgs=Module._getSizeHalSMArray(args);
        if (lArgs!=3) {return Module.HalSM.null;}
        var uidMessageVar=Module._getVariableFromHalSMArray(args,1);
        var newContentMessageVar=Module._getVariableFromHalSMArray(args,2);
        if (Module._getTypeVariable(Module._getVariableFromHalSMArray(args,0))===Module.HalSM.HalSMVariableType.HalSMCModule&&Module._getTypeVariable(uidMessageVar)===Module.HalSM.HalSMVariableType.int&&Module._getTypeVariable(newContentMessageVar)===Module.HalSM.HalSMVariableType.str) {
            Module.addContentMessage(Module._getLongIntFromValue(Module._getValueVariable(uidMessageVar)),CharArrayToString(Module._getStringFromValue(Module._getValueVariable(newContentMessageVar))));
        }
        return Module.HalSM.null;
    }
    
    static makeHTMLEvent(hsmc,args,vrs) {
        var lArgs=Module._getSizeHalSMArray(args);
        if (lArgs!=3) {return Module.HalSM.null;}
        var funcVar=Module._getVariableFromHalSMArray(args,1);
        var nameVar=Module._getVariableFromHalSMArray(args,2);
        if (Module._getTypeVariable(Module._getVariableFromHalSMArray(args,0))===Module.HalSM.HalSMVariableType.HalSMCModule&&Module._getTypeVariable(funcVar)===Module.HalSM.HalSMVariableType.HalSMLocalFunction&&Module._getTypeVariable(nameVar)===Module.HalSM.HalSMVariableType.str) {
            var name=CharArrayToString(Module._getStringFromValue(Module._getValueVariable(nameVar)));
            Module.htmlevents[name]=Module._getValueVariable(funcVar);
            return Module.ccall("stringToVar","number",["string"],["HalChatModule.runHTMLEvent('"+name+"',event,this)"]);
        }
        return Module.HalSM.null;
    }
    
    static runHTMLEvent(name,event,ths) {
        if (name in Module.htmlevents) {
            var argsH;
            var argss=Object.assign({},ths.dataset);
            if (argss.hasOwnProperty("eventdata")) {
                argsH=Module._initHalSMArray();
                Module._appendToHalSMArray(argsH,Module.eventDatas[parseInt(argss['eventdata'])]);
            } else {
                argsH=MainHalChatPlugins.getHalSMArguments([argss]);
            }
            Module._runLocalFunction(Module.htmlevents[name],argsH,Module.HalSM.nulldict);
        }
    }
    
    static openWindow(hsmc,args,vrs) {
        var lArgs=Module._getSizeHalSMArray(args);
        if (lArgs!=3) {return Module.HalSM.null;}
        //name: str, html: str
        var nameVar=Module._getVariableFromHalSMArray(args,1);
        var htmlVar=Module._getVariableFromHalSMArray(args,2);
        if (Module._getTypeVariable(Module._getVariableFromHalSMArray(args,0))===Module.HalSM.HalSMVariableType.HalSMCModule&&Module._getTypeVariable(nameVar)===Module.HalSM.HalSMVariableType.str&&Module._getTypeVariable(htmlVar)===Module.HalSM.HalSMVariableType.str) {
            Module.openWindow(CharArrayToString(Module._getStringFromValue(Module._getValueVariable(nameVar))),CharArrayToString(Module._getStringFromValue(Module._getValueVariable(htmlVar))));
        }
        return Module.HalSM.null;
    }
    
    static addStyle(hsmc,args,vrs) {
        var lArgs=Module._getSizeHalSMArray(args);
        if (lArgs!=2){return Module.HalSM.null;}
        //style: str
        var styleVar=Module._getVariableFromHalSMArray(args,1);
        if (Module._getTypeVariable(Module._getVariableFromHalSMArray(args,0))===Module.HalSM.HalSMVariableType.HalSMCModule&&Module._getTypeVariable(styleVar)===Module.HalSM.HalSMVariableType.str) {
            Module.styleElem.textContent=Module.styleElem.textContent+CharArrayToString(Module._getStringFromValue(Module._getValueVariable(styleVar)));
        }
        return Module.HalSM.null;
    }
    
    static eventData(hsmc,args,vrs) {
        var lArgs=Module._getSizeHalSMArray(args);
        if (lArgs!=2){return Module.HalSM.null;}
        //data: any
        var dataVar=Module._getVariableFromHalSMArray(args,1);
        if (Module._getTypeVariable(Module._getVariableFromHalSMArray(args,0))===Module.HalSM.HalSMVariableType.HalSMCModule) {
            var ind;
            for (var i=0;i<Module.eventDatas.length;i++) {
                if (Module.eventDatas[i]===-1) {
                    ind=i;
                    break;
                }
            }
            if (i===Module.eventDatas.length) {
                Module.eventDatas.push(dataVar);
                ind=i;
            } else {
                Module.eventDatas[i]=dataVar;
            }
            return Module.ccall("stringToVar","number",["string"],[ind.toString()]);
        }
        return Module.HalSM.null;
    }
    
    
    static async showMessage(hsmc,args,vrs) {
        var lArgs=Module._getSizeHalSMArray(args);
        if (lArgs!=2) {return Module.HalSM.null;}
        //text: str
        var textVar=Module._getVariableFromHalSMArray(args,1);
        if (Module._getTypeVariable(Module._getVariableFromHalSMArray(args,0))===Module.HalSM.HalSMVariableType.HalSMCModule&&Module._getTypeVariable(textVar)===Module.HalSM.HalSMVariableType.str) {
            await Module.showMessage(CharArrayToString(Module._getStringFromValue(Module._getValueVariable(textVar))));
        }
        return Module.HalSM.null;
    }
}

Module.HalSM.initialized=new Promise((resolve,reject)=>{
    Module.onRuntimeInitialized=async function() {
        Asyncify.handleSleep=function(startAsync) {
            assert(Asyncify.state !== Asyncify.State.Disabled, 'Asyncify cannot be done during or after the runtime exits');
            if (ABORT) return;
            err('ASYNCIFY: handleSleep ' + Asyncify.state);
            if (Asyncify.state === Asyncify.State.Normal) {
              // Prepare to sleep. Call startAsync, and see what happens:
              // if the code decided to call our callback synchronously,
              // then no async operation was in fact begun, and we don't
              // need to do anything.
              var reachedCallback = false;
              var reachedAfterCallback = false;
              startAsync((handleSleepReturnValue) => {
                if (ABORT) return;
                Asyncify.handleSleepReturnValue = handleSleepReturnValue || 0;
                reachedCallback = true;
                if (!reachedAfterCallback) {
                  // We are happening synchronously, so no need for async.
                  return;
                }
                // This async operation did not happen synchronously, so we did
                // unwind. In that case there can be no compiled code on the stack,
                // as it might break later operations (we can rewind ok now, but if
                // we unwind again, we would unwind through the extra compiled code
                // too).
                assert(!Asyncify.exportCallStack.length, 'Waking up (starting to rewind) must be done from JS, without compiled code on the stack.');
                err('ASYNCIFY: start rewind ' + Asyncify.currData);
                Asyncify.state = Asyncify.State.Rewinding;
                //runAndAbortIfError(() => Module['_asyncify_start_rewind'](Asyncify.currData));
                Module['_asyncify_start_rewind'](Asyncify.currData);
                if (typeof Browser != 'undefined' && Browser.mainLoop.func) {
                  Browser.mainLoop.resume();
                }
                var asyncWasmReturnValue, isError = false;
                try {
                  asyncWasmReturnValue = Asyncify.doRewind(Asyncify.currData);
                } catch (err) {
                  asyncWasmReturnValue = err;
                  isError = true;
                }
                // Track whether the return value was handled by any promise handlers.
                var handled = false;
                if (!Asyncify.currData) {
                  // All asynchronous execution has finished.
                  // `asyncWasmReturnValue` now contains the final
                  // return value of the exported async WASM function.
                  //
                  // Note: `asyncWasmReturnValue` is distinct from
                  // `Asyncify.handleSleepReturnValue`.
                  // `Asyncify.handleSleepReturnValue` contains the return
                  // value of the last C function to have executed
                  // `Asyncify.handleSleep()`, where as `asyncWasmReturnValue`
                  // contains the return value of the exported WASM function
                  // that may have called C functions that
                  // call `Asyncify.handleSleep()`.
                  var asyncPromiseHandlers = Asyncify.asyncPromiseHandlers;
                  if (asyncPromiseHandlers) {
                    Asyncify.asyncPromiseHandlers = null;
                    (isError ? asyncPromiseHandlers.reject : asyncPromiseHandlers.resolve)(asyncWasmReturnValue);
                    handled = true;
                  }
                }
                if (isError && !handled) {
                  // If there was an error and it was not handled by now, we have no choice but to
                  // rethrow that error into the global scope where it can be caught only by
                  // `onerror` or `onunhandledpromiserejection`.
                  throw asyncWasmReturnValue;
                }
              });
              reachedAfterCallback = true;
              if (!reachedCallback) {
                // A true async operation was begun; start a sleep.
                Asyncify.state = Asyncify.State.Unwinding;
                // TODO: reuse, don't alloc/free every sleep
                Asyncify.currData = Asyncify.allocateData();
                err('ASYNCIFY: start unwind ' + Asyncify.currData);
                runAndAbortIfError(() => Module['_asyncify_start_unwind'](Asyncify.currData));
                if (typeof Browser != 'undefined' && Browser.mainLoop.func) {
                  Browser.mainLoop.pause();
                }
              }
            } else if (Asyncify.state === Asyncify.State.Rewinding) {
              // Stop a resume.
              err('ASYNCIFY: stop rewind');
              Asyncify.state = Asyncify.State.Normal;
              runAndAbortIfError(Module['_asyncify_stop_rewind']);
              _free(Asyncify.currData);
              Asyncify.currData = null;
              // Call all sleep callbacks now that the sleep-resume is all done.
              Asyncify.sleepCallbacks.forEach((func) => callUserCallback(func));
            } else {
              abort('invalid state: ' + Asyncify.state);
            }
            return Asyncify.handleSleepReturnValue;
        }
        await Module.ccall("init","void",[],[],{async:true});
        Module.HalSM.null=Module._getHalSMNull();
        Module.HalSM.true=Module._getHalSMTrue();
        Module.HalSM.false=Module._getHalSMFalse();
        Module.HalSM.nulldict=Module._initDict();
        Module.HalSM.nullarray=Module._initHalSMArray();
        HalSMExecutable.addModule("HalChat",HalChatModule);
        //Module.ccall("stringToValue","number",["string"],["/resources/js/testhalchatplugin.hsme"])
        Module['HalSM']['initialized']=true;
        resolve();
    };
});

class HalSMExecutable {
    static addModule(name,mdl) {
        var funcs=Module._initDict();
        for (const [keyFunc,valFunc] of Object.entries(mdl.funcs)) {
            Module._putToDict(
                funcs,
                Module._initDictElement(
                    Module.ccall("stringToVar","number",["string"],[keyFunc]),
                    Module._variableInit(Module._initFunctionC(Module.addFunction(valFunc,"iiii")),Module.HalSM.HalSMVariableType.HalSMFunctionC)
                )
            );
        }
        
        var vars=Module._initDict();
        for (const [keyVar,valVar] of Object.entries(mdl.initializeVars())) {
            Module._putToDict(
                vars,
                Module._initDictElement(
                    Module.ccall("stringToVar","number",["string"],[keyVar]),
                    valVar
                )
            );
        }
        
        var classes=Module._initDict();
        for (const [keyCls,valCls] of Object.entries(mdl.clsses)) {
            Module._putToDict(
                classes,
                Module._initDictElement(
                    Module.ccall("stringToVar","number",["string"],[keyCls]),
                    Module._variableInit(HalSMScript.initHalSMClassC(keyCls,valCls),Module.HalSM.HalSMVariableType.HalSMClassC)
                )
            );
        }
        
        Module.ccall("addModule","void",["string","number","number","number"],[name,funcs,vars,classes]);
    }
}

async function startHalSMPlugins(addContentMessage,openWindow,showMessage) {
    Module.styleElem.id="styleElementHalSMPlugin";
    document.body.appendChild(Module.styleElem);
    Module.addContentMessage=addContentMessage;
    Module.openWindow=openWindow;
    Module.showMessage=showMessage;
    if (Module.HalSM.wasmInstalled===false) {
        await new Promise((resolve,reject)=>{
            var s=document.createElement("script");
            s.type='text/javascript';
            s.src='https://halchat.halwarsing.net/resources/js/HalSMExecutable.js';
            s.onload=s.onreadystatechange=function() {
                if (!this.readyState || this.readyState == 'complete') {
                    Module.HalSM.wasmInstalled=true;
                    resolve();
                }
            };
            document.head.appendChild(s);
        });
    }
}

async function testPlugins() {
    /*await startHalSMPlugins();
    await Module.HalSM.initialized;
    await MainHalChatPlugins.loadPlugin("/resources/js/testhalchatplugin.hsme");
    MainHalChatPlugins.runEvent("onUserSendMessage",[2,3.1,"heyo"]);*/
}
//testPlugins();