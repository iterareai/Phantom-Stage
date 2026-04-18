[README.md](https://github.com/user-attachments/files/26848278/README.md)
# Phantom-Stage# XTC Ambiophonics v2 — AU Plugin for macOS
## 4-Stage Crosstalk Cancellation for Audirvana

---

## What This Plugin Does

Crosstalk Cancellation (XTC) dramatically improves the 3D soundstage from two speakers by
injecting a phase-inverted, time-delayed copy of each channel into the opposite channel.
This cancels the "crosstalk" your left ear hears from the right speaker, and vice versa —
producing a much wider, more immersive stereo image (Ambiophonics technique).

**Controls:**
- **Spkr Spread** — the total angle between your two speakers as seen from your listening position.
  For example, if your left speaker is 20° to your left and your right speaker is 20° to your right,
  set this to 40°. Typical XTC setups use 20°–40°. Range: 20°–120°.
- **Cancellation** — strength of the XTC effect (start around 0.5–0.6)
- **HF Boost** — slight high-frequency lift on the cross-feed path (2–4 dB recommended)
- **Bass Boost** — low shelf boost at 70Hz to restore body that XTC can thin out (try 4–6 dB)
- **Output Gain** — compensate for any overall level change
- **Bypass** — A/B compare with and without XTC

---

## Step 1 — Create a Free GitHub Account

1. Go to https://github.com and click **Sign up**
2. Choose the **Free** plan
3. Verify your email

---

## Step 2 — Create a New Repository

1. Click the **+** icon (top right) → **New repository**
2. Name it: `xtc-plugin`
3. Set it to **Public**
4. Check **Add a README file**
5. Click **Create repository**

---

## Step 3 — Upload the Plugin Files

In your new repo, click **Add file → Upload files**, then upload ALL of these files
maintaining the folder structure:

```
CMakeLists.txt
Source/PluginProcessor.h
Source/PluginProcessor.cpp
Source/PluginEditor.h
Source/PluginEditor.cpp
```

Commit the files with message: `Initial commit`

---

## Step 3b — Create the Build Workflow

Because `.github/` is a hidden folder that macOS won't let you drag into GitHub:

1. In your repo click **Add file → Create new file**
2. In the filename box type exactly: `.github/workflows/build.yml`
   (GitHub will create the folder structure automatically as you type the slashes)
3. Paste in the contents of `build.yml` from the zip
4. Click **Commit new file**

---

## Step 4 — Run the Build

1. In your repo, click the **Actions** tab
2. You should see **"Build XTC AU Plugin (macOS)"** listed
3. If it hasn't started automatically, click it → **Run workflow** → **Run workflow**
4. Wait ~5–10 minutes for GitHub's Mac servers to compile it
5. When it shows a green ✅, click on the workflow run

---

## Step 5 — Download Your Plugin

1. At the bottom of the completed workflow run, find **Artifacts**
2. Click **XTC-Ambiophonics-Mac** to download the zip
3. Unzip it — you'll find `XTC Ambiophonics v2.component`

---

## Step 6 — Install the AU Plugin

1. Open **Finder**
2. Press **Cmd+Shift+G** and go to: `~/Library/Audio/Plug-Ins/Components/`
3. Copy `XTC Ambiophonics v2.component` into that folder
4. **Important on macOS:** Right-click the file → Open → to clear the Gatekeeper warning
   (since it's not signed with an Apple Developer certificate)

---

## Step 7 — Enable in Audirvana

1. Open **Audirvana**
2. Go to **Preferences → Audio → Audio Units**
3. Click **Rescan plugins** if needed
4. Find **XTC Ambiophonics v2** in the plugin list
5. Add it to your audio chain

---

## Calibration Tips

- Start with **Spkr Spread = 40°**, **Cancellation = 0.5**, **HF Boost = 3dB**, **Bass Boost = 4dB**
- Sit in the sweet spot between your speakers
- Set Spkr Spread to match your actual speaker placement as seen from your listening position
- Gradually adjust Cancellation until the stereo image "snaps" wider
- If it sounds phasey or hollow, reduce Cancellation slightly
- If it sounds thin or bass-light, increase Bass Boost by a few dB
- Use Output Gain to match the bypassed level for fair A/B comparison

---

## Troubleshooting

**Plugin doesn't appear in Audirvana:**
- Make sure the `.component` file is in `~/Library/Audio/Plug-Ins/Components/` (user library, not system)
- Run `auval -a` in Terminal to verify macOS can see it

**Build failed on GitHub Actions:**
- Check the Actions log for errors
- Most common issue: the `.github/workflows/build.yml` file didn't upload with the correct path
- Make sure the workflow file uses `runs-on: macos-14`

**Gatekeeper blocks it:**
- Right-click → Open (not double-click)
- Or in Terminal:
  `xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/"XTC Ambiophonics v2.component"`
