#!/usr/bin/env python3
"""
web/patch_main.py - Dynamic patcher for web/main.js.

1. Locates the Appearance settings block and injects the Desktop Mode toggle
   if not already present. Safe, idempotent, and resilient to upstream changes.
2. Injects the default download directory path (/storage/emulated/0/Download)
   into the folder picker dialog and home directory resolvers.
"""

import sys
import os
import re

def patch_desktop_mode(content):
    if "Desktop Mode" in content or "ag_desktop_mode" in content:
        print("[web-patch] Desktop Mode toggle already present in main.js")
        return content

    # 1. State hook injection pattern in Appearance component:
    hook_pattern = re.compile(r'(var\s+[A-Za-z0-9_$]+=[A-Za-z0-9_$]+!==3,\s*[A-Za-z0-9_$]+=[A-Za-z0-9_$]+!==2,\s*[A-Za-z0-9_$]+=[A-Za-z0-9_$]+=>\s*[A-Za-z0-9_$]+=>\{.*?\};)')
    hook_match = hook_pattern.search(content)

    # 2. JSX Theme element injection: find Theme element in Appearance
    theme_idx = content.find('{label:"Theme"')
    theme_match = None
    react_id = "A"
    if theme_idx != -1:
        window_start = max(0, theme_idx - 100)
        window_end = min(len(content), theme_idx + 500)
        window_text = content[window_start:window_end]
        theme_pattern = re.compile(
            r'([A-Za-z0-9_$]+)\.createElement\(([A-Za-z0-9_$]+),\{label:"Theme",.*?onSelect:[A-Za-z0-9_$]+\}\)'
        )
        theme_match = theme_pattern.search(window_text)
        if theme_match:
            react_id = theme_match.group(1)

    if not hook_match or not theme_match:
        print(f"[web-patch] Appearance patterns not matched: hook={bool(hook_match)}, theme={bool(theme_match)}", file=sys.stderr)
        return content

    orig_hook = hook_match.group(1)
    injected_hook = (
        f"var [isDesktop,setIsDesktop]=(0,{react_id}.useState)(()=>{{"
        "try{if(window.Android&&typeof window.Android.isDesktopMode==='function')return window.Android.isDesktopMode();"
        "var s=localStorage.getItem('ag_desktop_mode');return s===null?!1:s==='true'}catch(_){return !1}});"
        f"var onDesktopToggle=(0,{react_id}.useCallback)(v=>{{"
        "setIsDesktop(v);try{localStorage.setItem('ag_desktop_mode',String(v));"
        "if(window.Android&&typeof window.Android.setDesktopMode==='function')window.Android.setDesktopMode(v)}catch(_){}},[]);"
        + orig_hook
    )

    item_comp = "v3"
    switch_comp = "dX"
    re_idx = content.find('rightElement:')
    if re_idx != -1:
        re_window = content[max(0, re_idx - 150):min(len(content), re_idx + 150)]
        item_m = re.search(r'([A-Za-z0-9_$]+)\.createElement\(([A-Za-z0-9_$]+),\{label:[^,]+,description:[^,]+,rightElement:', re_window)
        if item_m:
            item_comp = item_m.group(2)
        sw_m = re.search(r'rightElement:[A-Za-z0-9_$]+\.createElement\(([A-Za-z0-9_$]+),\{checked:', re_window)
        if sw_m:
            switch_comp = sw_m.group(1)

    orig_theme = theme_match.group(0)
    injected_theme = (
        orig_theme
        + f',{react_id}.createElement({item_comp},{{label:"Desktop Mode",description:"Use desktop viewport and user agent for the workspace",'
        + f'rightElement:{react_id}.createElement({switch_comp},{{checked:isDesktop,onCheckedChange:onDesktopToggle}})}})'
    )

    content = content.replace(orig_hook, injected_hook, 1)
    content = content.replace(orig_theme, injected_theme, 1)
    print("[web-patch] Successfully patched Desktop Mode toggle into main.js")
    return content

def patch_downloads_directory(content):
    # 1. Patch wqc initialPath
    orig_wqc = 'initialPath:b?b.fsPath:ch?"C:/":"/"'
    new_wqc = 'initialPath:((window.__APP_CONFIG__&&window.__APP_CONFIG__.workspaceRoot)||"/storage/emulated/0/Download")'
    if orig_wqc in content:
        content = content.replace(orig_wqc, new_wqc, 1)
        print("[web-patch] Patched wqc interactive folder picker initialPath to default Downloads")
    else:
        print("[web-patch] orig_wqc not found or already patched")

    # 2. Patch iqb dialog component initialPath fallback
    orig_iqb = 'var iqb=({isOpen:a,initialPath:b,onSelectFolder:c,onClose:e,fetchDirectoryContents:f,statPath:g,createDirectory:h})=>{\n'
    if orig_iqb not in content:
        orig_iqb = 'var iqb=({isOpen:a,initialPath:b,onSelectFolder:c,onClose:e,fetchDirectoryContents:f,statPath:g,createDirectory:h})=>{'
    new_iqb = 'var iqb=({isOpen:a,initialPath:b,onSelectFolder:c,onClose:e,fetchDirectoryContents:f,statPath:g,createDirectory:h})=>{if(!b||b==="/data/user/0/com.agy/files"||b==="/data/user/0/com.agy/files/")b=(window.__APP_CONFIG__&&window.__APP_CONFIG__.workspaceRoot)||"/storage/emulated/0/Download";'
    if orig_iqb in content:
        content = content.replace(orig_iqb, new_iqb, 1)
        print("[web-patch] Patched iqb folder picker component to override /data/user/0/com.agy/files")
    else:
        print("[web-patch] orig_iqb not found or already patched")

    # 3. Patch Fzc homeDirUri
    orig_fzc = 'return{homeDirUri:Oe(e.homeDirUri),username:f}'
    new_fzc = 'return{homeDirUri:Oe((window.__APP_CONFIG__&&window.__APP_CONFIG__.workspaceRoot)?"file://"+window.__APP_CONFIG__.workspaceRoot:e.homeDirUri),username:f}'
    if orig_fzc in content:
        content = content.replace(orig_fzc, new_fzc, 1)
        print("[web-patch] Patched Fzc homeDirUri to point to default Downloads")
    else:
        print("[web-patch] orig_fzc not found or already patched")

    # 4. Patch getHomeDirUri
    orig_gh = 'async getHomeDirUri(){var a=d,b=d;try{let c=p(Fga,{});return b(await a(this.lsClient.getLocalUserInfo(c))).homeDirUri}finally{a()}}'
    new_gh = 'async getHomeDirUri(){return(window.__APP_CONFIG__&&window.__APP_CONFIG__.workspaceRoot)?"file://"+window.__APP_CONFIG__.workspaceRoot:"file:///storage/emulated/0/Download"}'
    if orig_gh in content:
        content = content.replace(orig_gh, new_gh, 1)
        print("[web-patch] Patched getHomeDirUri to return Downloads URI")
    else:
        print("[web-patch] orig_gh not found or already patched")

    return content

def patch_main(file_path):
    if not os.path.isfile(file_path):
        print(f"[web-patch] file not found: {file_path}", file=sys.stderr)
        return False

    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()

    content = patch_desktop_mode(content)
    content = patch_downloads_directory(content)

    with open(file_path, "w", encoding="utf-8") as f:
        f.write(content)

    return True

if __name__ == "__main__":
    path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "main.js")
    if not patch_main(path):
        sys.exit(1)
