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
import android.view.animation.DecelerateInterpolator;
import android.view.animation.OvershootInterpolator;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;

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

    private FrameLayout gameOverlay;

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

        String[] subLabels  = {"Inventory", "Search", "Rest", "Explore", "Settings"};
        int[] subKeyCodes   = {
            KeyEvent.KEYCODE_I, KeyEvent.KEYCODE_S, KeyEvent.KEYCODE_Z,
            KeyEvent.KEYCODE_X, KeyEvent.KEYCODE_ESCAPE
        };
        String[] subHints   = {"i", "s", "z", "x", "esc"};

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
