# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Uncomment this to preserve the line number information for
# debugging stack traces.
#-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile

# Игнорируем недостающие классы, если openjsse отсутствует
-dontwarn org.openjsse.**
-dontwarn org.bouncycastle.**
-dontwarn org.conscrypt.**

# JNI entry points and callbacks used by the embedded llama.cpp engine and HalSM.
-keep class halwarsing.net.halchatandroid.localai.LLamaNative { *; }
-keep interface halwarsing.net.halchatandroid.localai.LLamaNative$StreamCallback { *; }
-keep class halwarsing.net.halchatandroid.halsm.HalSMNative { *; }
-keep interface halwarsing.net.halchatandroid.halsm.HalSMNative$* { *; }
-keep class halwarsing.net.halchatandroid.halsm.HalSMExecutable { *; }
-keep class halwarsing.net.halchatandroid.halsm.HalSMExecutable$* { *; }
-keep class halwarsing.net.halchatandroid.halsm.HalSMException { *; }

# HalSM modules: HalSMExecutable.addModule() registers their static methods, nested classes and initializeVars() by name
-keepattributes InnerClasses,EnclosingMethod
-keep class halwarsing.net.halchatandroid.main.HalChatModule { *; }
-keep class halwarsing.net.halchatandroid.main.HalChatModule$* { *; }
-keep class halwarsing.net.halchatandroid.halsm.LocalLLMHalSM { *; }
-keep class halwarsing.net.halchatandroid.halsm.LocalLLMHalSM$* { *; }

-keepclassmembers class halwarsing.net.halchatandroid.plugins.PluginWebUi$Bridge {
    @android.webkit.JavascriptInterface <methods>;
}
