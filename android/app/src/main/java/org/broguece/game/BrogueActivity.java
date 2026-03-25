package org.broguece.game;

import android.animation.ArgbEvaluator;
import android.animation.ValueAnimator;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Shader;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.LayerDrawable;
import android.graphics.drawable.RippleDrawable;
import android.graphics.drawable.StateListDrawable;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.OvershootInterpolator;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import org.json.JSONArray;
import org.json.JSONObject;
import org.libsdl.app.SDLActivity;

public class BrogueActivity extends SDLActivity {

    // Brogue's palette — derived from the game's actual color definitions
    private static final int DEEP_INDIGO   = Color.argb(230, 18, 15, 38);   // interfaceBoxColor
    private static final int FLAME_EMBER   = Color.argb(255, 180, 100, 40); // warm flame highlight
    private static final int FLAME_DIM     = Color.argb(255, 100, 55, 20);  // subdued flame
    private static final int PALE_BLUE     = Color.argb(255, 140, 150, 190);// flameTitleColor text
    private static final int GHOST_WHITE   = Color.argb(255, 210, 205, 220);
    private static final int VOID_BLACK    = Color.argb(240, 8, 6, 16);
    private static final int SUBMENU_BG    = Color.argb(235, 12, 10, 25);
    private static final int RIPPLE_GLOW   = Color.argb(80, 180, 120, 50);
    private static final int BORDER_DIM    = Color.argb(120, 80, 65, 40);
    private static final int BORDER_ACTIVE = Color.argb(200, 180, 120, 50);

    private static final int INVENTORY_BG  = Color.argb(245, 10, 8, 22);
    private static final int ITEM_BG       = Color.argb(200, 20, 17, 42);
    private static final int EQUIPPED_GLOW = Color.argb(220, 45, 35, 55);
    private static final int ACTION_BG     = Color.argb(220, 30, 25, 55);

    private FrameLayout gameOverlay;
    private FrameLayout inventoryOverlay;
    private View currentlyExpandedDetail;

    @Override
    protected String[] getLibraries() {
        return new String[]{ "SDL2", "SDL2_image", "brogue" };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // ===== Game overlay =====
        gameOverlay = new FrameLayout(this);

        // Submenu
        LinearLayout submenu = new LinearLayout(this);
        submenu.setOrientation(LinearLayout.VERTICAL);
        submenu.setBackground(makeSubmenuBackground());
        submenu.setPadding(dpToPx(3), dpToPx(4), dpToPx(3), dpToPx(3));
        submenu.setVisibility(View.GONE);
        submenu.setElevation(dpToPx(8));

        Button actionsBtn = makeBarButton("Menu");

        String[] subLabels  = {"Inventory", "Search", "Rest", "Explore"};
        int[] subKeyCodes   = {
            KeyEvent.KEYCODE_I, KeyEvent.KEYCODE_S, KeyEvent.KEYCODE_Z,
            KeyEvent.KEYCODE_X
        };
        String[] subHints   = {"i", "s", "z", "x"};

        for (int i = 0; i < subLabels.length; i++) {
            View item = makeSubmenuItem(subLabels[i], subHints[i]);
            final int keyCode = subKeyCodes[i];
            item.setOnClickListener(v -> {
                sendKey(keyCode);
                collapseSubmenu(submenu, actionsBtn);
            });
            LinearLayout.LayoutParams p = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT);
            p.setMargins(0, dpToPx(1), 0, dpToPx(1));
            submenu.addView(item, p);
        }

        // Settings opens native panel
        View settingsItem = makeSubmenuItem("Settings", "");
        settingsItem.setOnClickListener(v -> {
            collapseSubmenu(submenu, actionsBtn);
            showSettingsPanel();
        });
        LinearLayout.LayoutParams settingsP = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT);
        settingsP.setMargins(0, dpToPx(4), 0, dpToPx(1));
        submenu.addView(settingsItem, settingsP);

        // Bottom bar
        LinearLayout bottomBar = new LinearLayout(this);
        bottomBar.setOrientation(LinearLayout.HORIZONTAL);
        bottomBar.setGravity(Gravity.END | Gravity.CENTER_VERTICAL);
        bottomBar.setBackground(makeBarBackground());
        int barPad = dpToPx(4);
        bottomBar.setPadding(barPad, barPad, barPad, barPad);

        actionsBtn.setOnClickListener(v -> {
            boolean open = submenu.getVisibility() == View.VISIBLE;
            if (open) {
                collapseSubmenu(submenu, actionsBtn);
            } else {
                expandSubmenu(submenu, actionsBtn);
            }
        });

        Button mouseBtn = makeBarButton("Mouse");
        mouseBtn.setOnClickListener(v -> {
            sendKey(KeyEvent.KEYCODE_MOVE_HOME);
            boolean active = v.getTag() != null && (boolean) v.getTag();
            active = !active;
            v.setTag(active);
            animateToggle(v, active);
        });

        Button clickBtn = makeBarButton("Click");
        clickBtn.setOnClickListener(v -> {
            sendKey(KeyEvent.KEYCODE_ENTER);
            mouseBtn.setTag(false);
            animateToggle(mouseBtn, false);
            pulseButton(v);
        });

        int btnWidth = dpToPx(76);
        int btnMargin = dpToPx(3);
        for (Button btn : new Button[]{mouseBtn, clickBtn, actionsBtn}) {
            LinearLayout.LayoutParams p = new LinearLayout.LayoutParams(
                btnWidth, dpToPx(40));
            p.setMargins(btnMargin, 0, btnMargin, 0);
            bottomBar.addView(btn, p);
        }

        // Stack submenu above bar
        LinearLayout bottomGroup = new LinearLayout(this);
        bottomGroup.setOrientation(LinearLayout.VERTICAL);
        bottomGroup.setGravity(Gravity.END);
        bottomGroup.addView(submenu, new LinearLayout.LayoutParams(
            dpToPx(170), LinearLayout.LayoutParams.WRAP_CONTENT));
        bottomGroup.addView(bottomBar, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.WRAP_CONTENT,
            LinearLayout.LayoutParams.WRAP_CONTENT));

        gameOverlay.addView(bottomGroup, new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.WRAP_CONTENT,
            FrameLayout.LayoutParams.WRAP_CONTENT,
            Gravity.BOTTOM | Gravity.END));

        gameOverlay.setVisibility(View.GONE);

        addContentView(gameOverlay, new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT));

        // ===== Inventory overlay (created once, populated dynamically) =====
        inventoryOverlay = new FrameLayout(this);
        inventoryOverlay.setVisibility(View.GONE);
        inventoryOverlay.setClickable(true); // absorb touches so they don't reach the game
        addContentView(inventoryOverlay, new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT));
    }

    // ---- Settings panel ----

    private void showSettingsPanel() {
        inventoryOverlay.removeAllViews();
        currentlyExpandedDetail = null;

        // Backdrop
        View backdrop = new View(this);
        backdrop.setBackgroundColor(Color.argb(140, 0, 0, 0));
        backdrop.setOnClickListener(v -> hideSettingsPanel());
        inventoryOverlay.addView(backdrop, new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT));
        backdrop.setAlpha(0f);
        backdrop.animate().alpha(1f).setDuration(280).start();

        // Panel
        LinearLayout panel = new LinearLayout(this);
        panel.setOrientation(LinearLayout.VERTICAL);
        int pad = dpToPx(10);
        panel.setPadding(pad, pad, pad, pad);

        GradientDrawable panelBg = new GradientDrawable();
        panelBg.setShape(GradientDrawable.RECTANGLE);
        panelBg.setCornerRadii(new float[]{
            dpToPx(4), dpToPx(4), 0, 0, 0, 0, dpToPx(4), dpToPx(4)});
        panelBg.setColor(INVENTORY_BG);
        panelBg.setStroke(1, BORDER_DIM);

        ScrollView scrollView = new ScrollView(this);
        scrollView.setBackground(panelBg);
        scrollView.addView(panel);

        // Header
        TextView header = new TextView(this);
        header.setText("SETTINGS");
        header.setTextColor(FLAME_EMBER);
        header.setTextSize(TypedValue.COMPLEX_UNIT_SP, 13);
        header.setTypeface(Typeface.MONOSPACE, Typeface.BOLD);
        header.setLetterSpacing(0.15f);
        header.setPadding(dpToPx(4), dpToPx(2), 0, dpToPx(4));
        panel.addView(header);

        View headerSep = new View(this);
        GradientDrawable sepGrad = new GradientDrawable(
            GradientDrawable.Orientation.LEFT_RIGHT,
            new int[]{ FLAME_DIM, FLAME_EMBER, FLAME_DIM });
        headerSep.setBackground(sepGrad);
        LinearLayout.LayoutParams hSepP = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 1);
        hSepP.setMargins(dpToPx(4), 0, dpToPx(4), dpToPx(8));
        panel.addView(headerSep, hSepP);

        // Actions section
        addSettingsAction(panel, "Rest Until Better", v -> {
            hideSettingsPanel();
            sendChar('Z');
        });
        addSettingsAction(panel, "Autopilot", v -> {
            hideSettingsPanel();
            sendChar('A');
        });

        // Separator
        addSettingsSeparator(panel);

        // Toggles section
        addSettingsAction(panel, "Hide Color Effects", v -> {
            hideSettingsPanel();
            sendChar('\\');
        });
        addSettingsAction(panel, "Display Stealth Range", v -> {
            hideSettingsPanel();
            sendChar(']');
        });
        addSettingsAction(panel, "Enable Graphics", v -> {
            hideSettingsPanel();
            sendChar('G');
        });

        // Separator
        addSettingsSeparator(panel);

        // Info section
        addSettingsAction(panel, "View Dungeon Seed", v -> {
            hideSettingsPanel();
            sendChar('~');
        });

        // Separator
        addSettingsSeparator(panel);

        // Exit
        addSettingsAction(panel, "Exit Game", v -> {
            hideSettingsPanel();
            finishAndRemoveTask();
        });

        int panelWidth = Math.min(dpToPx(280),
            (int)(getResources().getDisplayMetrics().widthPixels * 0.6f));

        FrameLayout.LayoutParams scrollParams = new FrameLayout.LayoutParams(
            panelWidth, FrameLayout.LayoutParams.WRAP_CONTENT,
            Gravity.BOTTOM | Gravity.END);
        scrollParams.setMargins(0, dpToPx(8), dpToPx(8), dpToPx(52));

        inventoryOverlay.addView(scrollView, scrollParams);
        inventoryOverlay.setVisibility(View.VISIBLE);

        // Slide-up animation
        scrollView.setTranslationY(dpToPx(40));
        scrollView.setAlpha(0f);
        scrollView.animate()
            .translationY(0).alpha(1f)
            .setDuration(200)
            .setInterpolator(new DecelerateInterpolator(1.5f))
            .start();
    }

    private void hideSettingsPanel() {
        if (inventoryOverlay.getChildCount() < 2) {
            inventoryOverlay.setVisibility(View.GONE);
            inventoryOverlay.removeAllViews();
            return;
        }
        View backdrop = inventoryOverlay.getChildAt(0);
        View panel = inventoryOverlay.getChildAt(1);

        panel.animate()
            .translationY(dpToPx(30)).alpha(0f)
            .setDuration(150)
            .setInterpolator(new DecelerateInterpolator())
            .start();

        backdrop.animate()
            .alpha(0f)
            .setDuration(150)
            .withEndAction(() -> {
                inventoryOverlay.setVisibility(View.GONE);
                inventoryOverlay.removeAllViews();
            })
            .start();
    }

    private void addSettingsAction(LinearLayout panel, String label, View.OnClickListener listener) {
        LinearLayout row = new LinearLayout(this);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(dpToPx(12), dpToPx(10), dpToPx(12), dpToPx(10));
        row.setMinimumHeight(dpToPx(44));

        GradientDrawable bg = new GradientDrawable();
        bg.setShape(GradientDrawable.RECTANGLE);
        bg.setCornerRadius(dpToPx(3));
        bg.setColor(ITEM_BG);
        row.setBackground(new RippleDrawable(
            ColorStateList.valueOf(RIPPLE_GLOW), bg, null));

        row.setOnTouchListener((v, e) -> {
            if (e.getAction() == MotionEvent.ACTION_DOWN) {
                v.animate().scaleX(0.98f).scaleY(0.98f).setDuration(60).start();
            } else if (e.getAction() == MotionEvent.ACTION_UP
                    || e.getAction() == MotionEvent.ACTION_CANCEL) {
                v.animate().scaleX(1f).scaleY(1f).setDuration(100).start();
            }
            return false;
        });

        TextView labelView = new TextView(this);
        labelView.setText(label);
        labelView.setTextColor(GHOST_WHITE);
        labelView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
        labelView.setTypeface(Typeface.MONOSPACE);
        row.addView(labelView, new LinearLayout.LayoutParams(
            0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f));

        row.setOnClickListener(listener);

        LinearLayout.LayoutParams p = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT);
        p.setMargins(0, dpToPx(2), 0, dpToPx(2));
        panel.addView(row, p);
    }

    private void addSettingsSeparator(LinearLayout panel) {
        View sep = new View(this);
        sep.setBackgroundColor(BORDER_DIM);
        LinearLayout.LayoutParams p = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 1);
        p.setMargins(dpToPx(4), dpToPx(4), dpToPx(4), dpToPx(4));
        panel.addView(sep, p);
    }

    // ---- Inventory UI ----

    public void showInventory(final String json) {
        runOnUiThread(() -> {
            inventoryOverlay.removeAllViews();
            currentlyExpandedDetail = null;

            try {
                JSONObject root = new JSONObject(json);
                String mode = root.optString("mode", "inventory");
                boolean selectMode = "select".equals(mode);
                JSONArray items = root.getJSONArray("items");

                int equippedCount = 0;
                if (items.length() > 0) {
                    equippedCount = items.getJSONObject(0).optInt("equippedCount", 0);
                }

                // Semi-transparent backdrop — tap to close, fades in
                View backdrop = new View(this);
                backdrop.setBackgroundColor(Color.argb(140, 0, 0, 0));
                backdrop.setOnClickListener(v -> sendKey(KeyEvent.KEYCODE_ESCAPE));
                inventoryOverlay.addView(backdrop, new FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.MATCH_PARENT,
                    FrameLayout.LayoutParams.MATCH_PARENT));
                backdrop.setAlpha(0f);
                backdrop.animate().alpha(1f).setDuration(280).start();

                // Inventory panel — right-aligned, scrollable
                ScrollView scrollView = new ScrollView(this);
                scrollView.setFillViewport(false);
                scrollView.setOverScrollMode(View.OVER_SCROLL_NEVER);
                scrollView.setFadingEdgeLength(dpToPx(24));
                scrollView.setVerticalFadingEdgeEnabled(true);

                LinearLayout panel = new LinearLayout(this);
                panel.setOrientation(LinearLayout.VERTICAL);
                int pad = dpToPx(8);
                panel.setPadding(pad, pad, pad, pad);

                GradientDrawable panelBg = new GradientDrawable();
                panelBg.setShape(GradientDrawable.RECTANGLE);
                panelBg.setCornerRadii(new float[]{
                    dpToPx(4), dpToPx(4), 0, 0, 0, 0, dpToPx(4), dpToPx(4)});
                panelBg.setColor(INVENTORY_BG);
                panelBg.setStroke(1, BORDER_DIM);
                scrollView.setBackground(panelBg);

                // Header
                TextView header = new TextView(this);
                header.setText(selectMode ? "SELECT ITEM" : "INVENTORY");
                header.setTextColor(selectMode ? PALE_BLUE : FLAME_EMBER);
                header.setTextSize(TypedValue.COMPLEX_UNIT_SP, 13);
                header.setTypeface(Typeface.MONOSPACE, Typeface.BOLD);
                header.setLetterSpacing(0.15f);
                header.setPadding(dpToPx(4), dpToPx(2), 0, dpToPx(4));
                panel.addView(header);

                // Header separator
                View headerSep = new View(this);
                GradientDrawable sepGrad = new GradientDrawable(
                    GradientDrawable.Orientation.LEFT_RIGHT,
                    new int[]{ FLAME_DIM, FLAME_EMBER, FLAME_DIM });
                headerSep.setBackground(sepGrad);
                LinearLayout.LayoutParams hSepP = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT, 1);
                hSepP.setMargins(dpToPx(4), 0, dpToPx(4), dpToPx(6));
                panel.addView(headerSep, hSepP);

                if (items.length() == 0) {
                    TextView emptyMsg = new TextView(this);
                    emptyMsg.setText("Your pack is empty.");
                    emptyMsg.setTextColor(Color.argb(160, 140, 150, 190));
                    emptyMsg.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
                    emptyMsg.setTypeface(Typeface.MONOSPACE, Typeface.ITALIC);
                    emptyMsg.setPadding(dpToPx(8), dpToPx(16), dpToPx(8), dpToPx(16));
                    emptyMsg.setGravity(Gravity.CENTER);
                    panel.addView(emptyMsg);
                }

                for (int i = 0; i < items.length(); i++) {
                    JSONObject item = items.getJSONObject(i);

                    // Separator between equipped and unequipped
                    if (i == equippedCount && equippedCount > 0) {
                        View sep = new View(this);
                        sep.setBackgroundColor(BORDER_DIM);
                        LinearLayout.LayoutParams sepP = new LinearLayout.LayoutParams(
                            LinearLayout.LayoutParams.MATCH_PARENT, 1);
                        sepP.setMargins(dpToPx(4), dpToPx(4), dpToPx(4), dpToPx(4));
                        panel.addView(sep, sepP);
                    }

                    panel.addView(selectMode
                        ? makeSelectRow(item)
                        : makeInventoryRow(item));
                }

                scrollView.addView(panel);

                int panelWidth = Math.min(dpToPx(340),
                    (int)(getResources().getDisplayMetrics().widthPixels * 0.75f));

                FrameLayout.LayoutParams scrollParams = new FrameLayout.LayoutParams(
                    panelWidth, FrameLayout.LayoutParams.MATCH_PARENT,
                    Gravity.END);
                scrollParams.setMargins(0, dpToPx(8), 0, dpToPx(52));

                inventoryOverlay.addView(scrollView, scrollParams);
                inventoryOverlay.setVisibility(View.VISIBLE);

                // Slide-in animation
                scrollView.setTranslationX(panelWidth);
                scrollView.animate()
                    .translationX(0)
                    .setDuration(280)
                    .setInterpolator(new DecelerateInterpolator(1.5f))
                    .start();

            } catch (Exception e) {
                sendKey(KeyEvent.KEYCODE_ESCAPE);
            }
        });
    }

    public void hideInventory() {
        runOnUiThread(() -> {
            currentlyExpandedDetail = null;
            if (inventoryOverlay.getChildCount() < 2) {
                inventoryOverlay.setVisibility(View.GONE);
                inventoryOverlay.removeAllViews();
                return;
            }
            View backdrop = inventoryOverlay.getChildAt(0);
            View scrollView = inventoryOverlay.getChildAt(1);

            scrollView.animate()
                .translationX(scrollView.getWidth())
                .setDuration(200)
                .setInterpolator(new DecelerateInterpolator())
                .start();

            backdrop.animate()
                .alpha(0f)
                .setDuration(200)
                .withEndAction(() -> {
                    inventoryOverlay.setVisibility(View.GONE);
                    inventoryOverlay.removeAllViews();
                })
                .start();
        });
    }

    private View makeInventoryRow(JSONObject item) {
        LinearLayout row = new LinearLayout(this);
        row.setOrientation(LinearLayout.VERTICAL);

        boolean equipped = item.optBoolean("equipped", false);
        boolean selectable = item.optBoolean("selectable", true);
        char letter = item.optString("letter", "?").charAt(0);
        String name = item.optString("name", "???");
        String desc = item.optString("desc", "");
        String actions = item.optString("actions", "");

        // Item header row (letter + name + chevron)
        LinearLayout headerRow = new LinearLayout(this);
        headerRow.setOrientation(LinearLayout.HORIZONTAL);
        headerRow.setGravity(Gravity.CENTER_VERTICAL);
        headerRow.setPadding(dpToPx(8), dpToPx(8), dpToPx(8), dpToPx(8));
        headerRow.setMinimumHeight(dpToPx(44));

        GradientDrawable rowBg = new GradientDrawable();
        rowBg.setShape(GradientDrawable.RECTANGLE);
        rowBg.setCornerRadius(dpToPx(3));
        rowBg.setColor(equipped ? EQUIPPED_GLOW : ITEM_BG);
        if (equipped) {
            rowBg.setStroke(dpToPx(1), FLAME_DIM);
        }
        headerRow.setBackground(new RippleDrawable(
            ColorStateList.valueOf(RIPPLE_GLOW), rowBg, null));

        // Touch animation on header row
        headerRow.setOnTouchListener((v, e) -> {
            if (e.getAction() == MotionEvent.ACTION_DOWN) {
                v.animate().scaleX(0.98f).scaleY(0.98f).setDuration(60).start();
            } else if (e.getAction() == MotionEvent.ACTION_UP
                    || e.getAction() == MotionEvent.ACTION_CANCEL) {
                v.animate().scaleX(1f).scaleY(1f).setDuration(100).start();
            }
            return false;
        });

        // Letter badge
        TextView letterView = new TextView(this);
        letterView.setText(String.valueOf(letter));
        letterView.setTextColor(FLAME_EMBER);
        letterView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
        letterView.setTypeface(Typeface.MONOSPACE, Typeface.BOLD);
        letterView.setGravity(Gravity.CENTER);
        letterView.setMinWidth(dpToPx(24));
        headerRow.addView(letterView);

        // Item name
        TextView nameView = new TextView(this);
        String displayName = name + (equipped ? " (equipped)" : "");
        nameView.setText(displayName);
        nameView.setTextColor(selectable ? GHOST_WHITE : Color.argb(150, 120, 115, 130));
        nameView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
        nameView.setTypeface(Typeface.MONOSPACE);
        nameView.setPadding(dpToPx(6), 0, 0, 0);
        nameView.setMaxLines(1);
        nameView.setEllipsize(android.text.TextUtils.TruncateAt.END);
        LinearLayout.LayoutParams nameP = new LinearLayout.LayoutParams(
            0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f);
        headerRow.addView(nameView, nameP);

        // Expand/collapse indicator
        TextView chevron = new TextView(this);
        chevron.setText("\u25BE"); // ▾
        chevron.setTextColor(Color.argb(120, 140, 150, 190));
        chevron.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
        chevron.setPadding(dpToPx(4), 0, 0, 0);
        headerRow.addView(chevron);

        headerRow.setContentDescription(displayName);
        row.addView(headerRow);

        // Expandable detail section (initially hidden)
        LinearLayout detailSection = new LinearLayout(this);
        detailSection.setOrientation(LinearLayout.VERTICAL);
        detailSection.setVisibility(View.GONE);
        detailSection.setPadding(dpToPx(12), dpToPx(4), dpToPx(8), dpToPx(8));

        GradientDrawable detailBg = new GradientDrawable();
        detailBg.setShape(GradientDrawable.RECTANGLE);
        detailBg.setCornerRadii(new float[]{
            0, 0, 0, 0, dpToPx(3), dpToPx(3), dpToPx(3), dpToPx(3)});
        detailBg.setColor(Color.argb(180, 15, 12, 32));
        detailSection.setBackground(detailBg);

        // Description text
        if (!desc.isEmpty()) {
            TextView descView = new TextView(this);
            descView.setText(desc);
            descView.setTextColor(PALE_BLUE);
            descView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 11);
            descView.setTypeface(Typeface.MONOSPACE);
            descView.setLineSpacing(0, 1.3f);
            descView.setPadding(0, dpToPx(4), 0, dpToPx(8));
            detailSection.addView(descView);
        }

        // Action buttons
        if (!actions.isEmpty()) {
            LinearLayout actionRow = new LinearLayout(this);
            actionRow.setOrientation(LinearLayout.HORIZONTAL);
            actionRow.setGravity(Gravity.START);

            String[] actionKeys = actions.split(",");
            for (String ak : actionKeys) {
                if (ak.isEmpty()) continue;
                char actionChar = ak.charAt(0);
                String label = actionLabel(actionChar);

                Button actionBtn = new Button(this);
                actionBtn.setText(label);
                actionBtn.setTextColor(GHOST_WHITE);
                actionBtn.setTextSize(TypedValue.COMPLEX_UNIT_SP, 11);
                actionBtn.setTypeface(Typeface.MONOSPACE);
                actionBtn.setAllCaps(true);
                actionBtn.setStateListAnimator(null);
                actionBtn.setElevation(0);
                actionBtn.setPadding(dpToPx(12), dpToPx(8), dpToPx(12), dpToPx(8));
                actionBtn.setMinWidth(0);
                actionBtn.setMinimumWidth(0);
                actionBtn.setMinHeight(dpToPx(36));
                actionBtn.setMinimumHeight(dpToPx(36));
                actionBtn.setContentDescription(label + " " + name);

                GradientDrawable abg = new GradientDrawable();
                abg.setShape(GradientDrawable.RECTANGLE);
                abg.setCornerRadius(dpToPx(3));
                abg.setColor(ACTION_BG);
                abg.setStroke(1, BORDER_DIM);
                actionBtn.setBackground(new RippleDrawable(
                    ColorStateList.valueOf(RIPPLE_GLOW), abg, null));

                actionBtn.setOnTouchListener((v, e) -> {
                    if (e.getAction() == MotionEvent.ACTION_DOWN) {
                        v.animate().scaleX(0.94f).scaleY(0.94f).setDuration(60).start();
                    } else if (e.getAction() == MotionEvent.ACTION_UP
                            || e.getAction() == MotionEvent.ACTION_CANCEL) {
                        v.animate().scaleX(1f).scaleY(1f).setDuration(100)
                            .setInterpolator(new OvershootInterpolator(2f)).start();
                    }
                    return false;
                });

                // Send item letter first, then action key
                final char itemLetter = letter;
                final char actionKey = actionChar;
                actionBtn.setOnClickListener(v -> {
                    sendChar(itemLetter);
                    v.postDelayed(() -> sendChar(actionKey), 50);
                });

                LinearLayout.LayoutParams btnP = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT);
                btnP.setMargins(0, 0, dpToPx(4), 0);
                actionRow.addView(actionBtn, btnP);
            }
            detailSection.addView(actionRow);
        }

        row.addView(detailSection);

        // Tap header to expand/collapse (accordion — only one open at a time)
        headerRow.setOnClickListener(v -> {
            if (detailSection.getVisibility() == View.GONE) {
                // Collapse previously expanded row
                if (currentlyExpandedDetail != null
                        && currentlyExpandedDetail != detailSection) {
                    View prev = currentlyExpandedDetail;
                    prev.animate().alpha(0f).setDuration(120)
                        .withEndAction(() -> prev.setVisibility(View.GONE)).start();
                }
                currentlyExpandedDetail = detailSection;
                detailSection.setVisibility(View.VISIBLE);
                detailSection.setAlpha(0f);
                detailSection.animate().alpha(1f).setDuration(150).start();
                chevron.setText("\u25B4"); // ▴
            } else {
                detailSection.animate().alpha(0f).setDuration(120)
                    .withEndAction(() -> detailSection.setVisibility(View.GONE)).start();
                currentlyExpandedDetail = null;
                chevron.setText("\u25BE"); // ▾
            }
        });

        // Outer margin
        LinearLayout.LayoutParams rowP = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT);
        rowP.setMargins(0, dpToPx(2), 0, dpToPx(2));
        row.setLayoutParams(rowP);

        return row;
    }

    // Simplified row for item selection mode — tap sends just the letter.
    private View makeSelectRow(JSONObject item) {
        boolean selectable = item.optBoolean("selectable", true);
        boolean equipped = item.optBoolean("equipped", false);
        char letter = item.optString("letter", "?").charAt(0);
        String name = item.optString("name", "???");

        LinearLayout row = new LinearLayout(this);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(dpToPx(8), dpToPx(8), dpToPx(8), dpToPx(8));
        row.setMinimumHeight(dpToPx(44));

        GradientDrawable rowBg = new GradientDrawable();
        rowBg.setShape(GradientDrawable.RECTANGLE);
        rowBg.setCornerRadius(dpToPx(3));
        rowBg.setColor(selectable ? ITEM_BG : Color.argb(100, 15, 12, 30));
        row.setBackground(new RippleDrawable(
            ColorStateList.valueOf(RIPPLE_GLOW), rowBg, null));

        // Letter badge
        TextView letterView = new TextView(this);
        letterView.setText(String.valueOf(letter));
        letterView.setTextColor(selectable ? FLAME_EMBER : FLAME_DIM);
        letterView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
        letterView.setTypeface(Typeface.MONOSPACE, Typeface.BOLD);
        letterView.setGravity(Gravity.CENTER);
        letterView.setMinWidth(dpToPx(24));
        row.addView(letterView);

        // Item name
        TextView nameView = new TextView(this);
        String displayName = name + (equipped ? " (equipped)" : "");
        nameView.setText(displayName);
        nameView.setTextColor(selectable ? GHOST_WHITE : Color.argb(100, 120, 115, 130));
        nameView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
        nameView.setTypeface(Typeface.MONOSPACE);
        nameView.setPadding(dpToPx(6), 0, 0, 0);
        nameView.setMaxLines(1);
        nameView.setEllipsize(android.text.TextUtils.TruncateAt.END);
        LinearLayout.LayoutParams nameP = new LinearLayout.LayoutParams(
            0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f);
        row.addView(nameView, nameP);

        row.setContentDescription(displayName);

        if (selectable) {
            row.setOnTouchListener((v, e) -> {
                if (e.getAction() == MotionEvent.ACTION_DOWN) {
                    v.animate().scaleX(0.98f).scaleY(0.98f).setDuration(60).start();
                } else if (e.getAction() == MotionEvent.ACTION_UP
                        || e.getAction() == MotionEvent.ACTION_CANCEL) {
                    v.animate().scaleX(1f).scaleY(1f).setDuration(100).start();
                }
                return false;
            });
            row.setOnClickListener(v -> sendChar(letter));
        } else {
            row.setClickable(false);
        }

        LinearLayout.LayoutParams rowP = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT);
        rowP.setMargins(0, dpToPx(2), 0, dpToPx(2));
        row.setLayoutParams(rowP);

        return row;
    }

    private String actionLabel(char key) {
        switch (key) {
            case 'a': return "Apply";
            case 'e': return "Equip";
            case 'r': return "Remove";
            case 'd': return "Drop";
            case 't': return "Throw";
            default:  return String.valueOf(key);
        }
    }

    // Send a character as an SDL text input event via key dispatch.
    // For letters a-z, we use the corresponding KEYCODE_A..Z.
    private void sendChar(char c) {
        if (c >= 'a' && c <= 'z') {
            sendKey(KeyEvent.KEYCODE_A + (c - 'a'));
        } else if (c >= 'A' && c <= 'Z') {
            sendKeyWithShift(KeyEvent.KEYCODE_A + (c - 'A'));
        } else {
            // Map symbols to their keycodes, with shift where needed
            switch (c) {
                case '\\': sendKey(KeyEvent.KEYCODE_BACKSLASH); break;
                case ']':  sendKey(KeyEvent.KEYCODE_RIGHT_BRACKET); break;
                case '[':  sendKey(KeyEvent.KEYCODE_LEFT_BRACKET); break;
                case '~':  sendKeyWithShift(KeyEvent.KEYCODE_GRAVE); break;
                case '`':  sendKey(KeyEvent.KEYCODE_GRAVE); break;
                case '-':  sendKey(KeyEvent.KEYCODE_MINUS); break;
                case '=':  sendKey(KeyEvent.KEYCODE_EQUALS); break;
                case ' ':  sendKey(KeyEvent.KEYCODE_SPACE); break;
                default:   sendKey(KeyEvent.KEYCODE_ESCAPE); break;
            }
        }
    }

    private void sendKeyWithShift(int keyCode) {
        long now = android.os.SystemClock.uptimeMillis();
        dispatchKeyEvent(new KeyEvent(now, now, KeyEvent.ACTION_DOWN, keyCode,
            0, KeyEvent.META_SHIFT_LEFT_ON));
        dispatchKeyEvent(new KeyEvent(now, now, KeyEvent.ACTION_UP, keyCode,
            0, KeyEvent.META_SHIFT_LEFT_ON));
    }

    public void setOverlayVisible(final boolean visible) {
        runOnUiThread(() -> {
            gameOverlay.setVisibility(visible ? View.VISIBLE : View.GONE);
        });
    }

    // ---- Bottom bar buttons ----

    private Button makeBarButton(String label) {
        Button btn = new Button(this);
        btn.setText(label);
        btn.setTextColor(PALE_BLUE);
        btn.setTextSize(TypedValue.COMPLEX_UNIT_SP, 11);
        btn.setTypeface(Typeface.MONOSPACE);
        btn.setLetterSpacing(0.05f);
        btn.setAllCaps(true);
        btn.setStateListAnimator(null);
        btn.setElevation(0);
        btn.setPadding(dpToPx(2), 0, dpToPx(2), 0);

        GradientDrawable bg = new GradientDrawable();
        bg.setShape(GradientDrawable.RECTANGLE);
        bg.setCornerRadius(dpToPx(2));
        bg.setColor(DEEP_INDIGO);
        bg.setStroke(1, BORDER_DIM);

        btn.setBackground(new RippleDrawable(
            ColorStateList.valueOf(RIPPLE_GLOW), bg, null));

        btn.setOnTouchListener((v, e) -> {
            if (e.getAction() == MotionEvent.ACTION_DOWN) {
                v.animate().scaleX(0.92f).scaleY(0.92f).setDuration(60).start();
            } else if (e.getAction() == MotionEvent.ACTION_UP
                    || e.getAction() == MotionEvent.ACTION_CANCEL) {
                v.animate().scaleX(1f).scaleY(1f).setDuration(120)
                    .setInterpolator(new OvershootInterpolator(2.5f)).start();
            }
            return false;
        });

        return btn;
    }

    // ---- Submenu items: label on left, key hint on right ----

    private View makeSubmenuItem(String label, String hint) {
        LinearLayout row = new LinearLayout(this);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(dpToPx(12), dpToPx(6), dpToPx(10), dpToPx(6));
        row.setMinimumHeight(dpToPx(40));
        row.setClickable(true);
        row.setFocusable(true);

        GradientDrawable bg = new GradientDrawable();
        bg.setShape(GradientDrawable.RECTANGLE);
        bg.setCornerRadius(dpToPx(2));
        bg.setColor(Color.TRANSPARENT);

        row.setBackground(new RippleDrawable(
            ColorStateList.valueOf(RIPPLE_GLOW), bg, null));

        row.setOnTouchListener((v, e) -> {
            if (e.getAction() == MotionEvent.ACTION_DOWN) {
                v.animate().translationX(dpToPx(2)).setDuration(60).start();
            } else if (e.getAction() == MotionEvent.ACTION_UP
                    || e.getAction() == MotionEvent.ACTION_CANCEL) {
                v.animate().translationX(0).setDuration(100).start();
            }
            return false;
        });

        TextView labelView = new TextView(this);
        labelView.setText(label);
        labelView.setTextColor(GHOST_WHITE);
        labelView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 13);
        labelView.setTypeface(Typeface.MONOSPACE);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
            0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f);
        row.addView(labelView, lp);

        TextView hintView = new TextView(this);
        hintView.setText(hint);
        hintView.setTextColor(FLAME_DIM);
        hintView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 10);
        hintView.setTypeface(Typeface.MONOSPACE);
        row.addView(hintView);

        return row;
    }

    // ---- Drawables ----

    private GradientDrawable makeSubmenuBackground() {
        GradientDrawable bg = new GradientDrawable();
        bg.setShape(GradientDrawable.RECTANGLE);
        bg.setCornerRadius(dpToPx(3));
        bg.setColor(SUBMENU_BG);
        bg.setStroke(1, BORDER_DIM);
        return bg;
    }

    private GradientDrawable makeBarBackground() {
        GradientDrawable bg = new GradientDrawable(
            GradientDrawable.Orientation.LEFT_RIGHT,
            new int[]{ Color.TRANSPARENT, VOID_BLACK, VOID_BLACK });
        bg.setCornerRadius(0);
        return bg;
    }

    // ---- Animations ----

    private void expandSubmenu(View submenu, View toggle) {
        submenu.setAlpha(0f);
        submenu.setTranslationY(dpToPx(8));
        submenu.setVisibility(View.VISIBLE);
        submenu.animate()
            .alpha(1f).translationY(0)
            .setDuration(150)
            .setInterpolator(new DecelerateInterpolator())
            .start();
        animateToggle(toggle, true);
    }

    private void collapseSubmenu(View submenu, View toggle) {
        submenu.animate()
            .alpha(0f).translationY(dpToPx(6))
            .setDuration(100)
            .withEndAction(() -> submenu.setVisibility(View.GONE))
            .start();
        animateToggle(toggle, false);
    }

    private void animateToggle(View v, boolean active) {
        GradientDrawable bg = extractBackground(v);
        if (bg == null) return;

        int from = active ? DEEP_INDIGO : Color.argb(200, 50, 35, 15);
        int to   = active ? Color.argb(200, 50, 35, 15) : DEEP_INDIGO;
        ValueAnimator anim = ValueAnimator.ofObject(new ArgbEvaluator(), from, to);
        anim.setDuration(150);
        anim.addUpdateListener(a -> bg.setColor((int) a.getAnimatedValue()));
        anim.start();

        bg.setStroke(1, active ? BORDER_ACTIVE : BORDER_DIM);

        if (v instanceof TextView) {
            ((TextView) v).setTextColor(active ? FLAME_EMBER : PALE_BLUE);
        }
    }

    private void pulseButton(View v) {
        v.animate().scaleX(1.08f).scaleY(1.08f).setDuration(80)
            .withEndAction(() ->
                v.animate().scaleX(1f).scaleY(1f).setDuration(120)
                    .setInterpolator(new OvershootInterpolator(3f)).start()
            ).start();
    }

    private GradientDrawable extractBackground(View v) {
        if (v.getBackground() instanceof RippleDrawable) {
            RippleDrawable rd = (RippleDrawable) v.getBackground();
            if (rd.getNumberOfLayers() > 0
                    && rd.getDrawable(0) instanceof GradientDrawable) {
                return (GradientDrawable) rd.getDrawable(0);
            }
        }
        return null;
    }

    // ---- Utilities ----

    private void sendKey(int keyCode) {
        dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, keyCode));
        dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_UP, keyCode));
    }

    private int dpToPx(int dp) {
        return (int) (dp * getResources().getDisplayMetrics().density);
    }
}
