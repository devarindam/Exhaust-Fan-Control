#ifndef WEB_PAGE_H
#define WEB_PAGE_H

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="description" content="Smart Fan Controller — Premium IoT Dashboard for ESP32-based temperature-controlled fan system.">
    <title>Smart Fan Controller</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700;800&display=swap" rel="stylesheet">
    <style>
        *, *::before, *::after {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        :root {
            --bg-primary: #0b0f1e;
            --bg-secondary: #131832;
            --bg-tertiary: #0d1225;
            --accent: #00d4ff;
            --accent-glow: rgba(0, 212, 255, 0.3);
            --fan-on: #00e676;
            --fan-on-glow: rgba(0, 230, 118, 0.4);
            --fan-off: #555;
            --card-bg: rgba(255, 255, 255, 0.03);
            --card-border: rgba(255, 255, 255, 0.08);
            --card-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
            --text-primary: #ffffff;
            --text-secondary: rgba(255, 255, 255, 0.7);
            --text-muted: rgba(255, 255, 255, 0.4);
            --danger: #ff5252;
            --success: #00e676;
        }

        body {
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
            background: linear-gradient(135deg, var(--bg-primary) 0%, var(--bg-secondary) 50%, var(--bg-tertiary) 100%);
            color: var(--text-primary);
            min-height: 100vh;
            overflow-x: hidden;
            -webkit-font-smoothing: antialiased;
            -moz-osx-font-smoothing: grayscale;
        }

        .container {
            max-width: 960px;
            margin: 0 auto;
            padding: 24px 20px 40px;
        }

        /* ── Header ─────────────────────────────────── */
        header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 18px 28px;
            background: var(--card-bg);
            border: 1px solid var(--card-border);
            border-radius: 20px;
            backdrop-filter: blur(20px);
            -webkit-backdrop-filter: blur(20px);
            box-shadow: var(--card-shadow);
            margin-bottom: 24px;
            animation: fadeSlideDown 0.6s ease-out;
        }

        .header-title {
            display: flex;
            align-items: center;
            gap: 12px;
            font-size: 18px;
            font-weight: 600;
            letter-spacing: -0.3px;
        }

        .header-title span.icon {
            font-size: 22px;
        }

        .connection-status {
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 13px;
            font-weight: 500;
            color: var(--text-secondary);
        }

        .status-dot {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background: var(--danger);
            transition: background 0.3s ease, box-shadow 0.3s ease;
        }

        .status-dot.connected {
            background: var(--success);
            box-shadow: 0 0 8px var(--fan-on-glow), 0 0 16px var(--fan-on-glow);
            animation: pulseGreen 2s ease-in-out infinite;
        }

        /* ── Main Grid ──────────────────────────────── */
        .dashboard-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 24px;
            margin-bottom: 24px;
        }

        /* ── Card Base ──────────────────────────────── */
        .card {
            background: var(--card-bg);
            border: 1px solid var(--card-border);
            border-radius: 20px;
            backdrop-filter: blur(20px);
            -webkit-backdrop-filter: blur(20px);
            box-shadow: var(--card-shadow);
            padding: 32px;
            animation: fadeSlideUp 0.7s ease-out both;
        }

        .card:nth-child(1) { animation-delay: 0.1s; }
        .card:nth-child(2) { animation-delay: 0.2s; }

        .card-full {
            animation-delay: 0.3s;
        }

        .card-label {
            font-size: 11px;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 2px;
            color: var(--text-muted);
            margin-bottom: 16px;
        }

        /* ── Temperature Card ───────────────────────── */
        .temp-card {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            text-align: center;
        }

        .temp-value {
            font-size: 68px;
            font-weight: 700;
            line-height: 1;
            background: linear-gradient(135deg, #00d4ff, #00e676);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            transition: all 0.5s ease;
            margin-bottom: 4px;
        }

        .temp-unit {
            font-size: 28px;
            font-weight: 300;
            opacity: 0.6;
        }

        .humidity-row {
            display: flex;
            align-items: center;
            gap: 8px;
            margin-top: 20px;
            font-size: 16px;
            font-weight: 500;
            color: var(--text-secondary);
        }

        .humidity-row .icon {
            font-size: 18px;
        }

        .humidity-bar-container {
            width: 100%;
            margin-top: 12px;
        }

        .humidity-bar-bg {
            width: 100%;
            height: 6px;
            border-radius: 3px;
            background: rgba(255, 255, 255, 0.08);
            overflow: hidden;
        }

        .humidity-bar-fill {
            height: 100%;
            border-radius: 3px;
            background: linear-gradient(90deg, #00d4ff, #00e676);
            transition: width 0.8s ease;
            width: 0%;
        }

        /* ── Temperature Gradient Bar ───────────────── */
        .temp-bar-container {
            width: 100%;
            margin-top: 20px;
            position: relative;
        }

        .temp-bar-label {
            font-size: 10px;
            text-transform: uppercase;
            letter-spacing: 1.5px;
            color: var(--text-muted);
            margin-bottom: 8px;
        }

        .temp-bar {
            width: 100%;
            height: 8px;
            border-radius: 4px;
            background: linear-gradient(90deg, #2196f3 0%, #4caf50 33%, #ff9800 66%, #f44336 100%);
            position: relative;
        }

        .temp-bar-marker {
            position: absolute;
            top: 50%;
            transform: translate(-50%, -50%);
            width: 16px;
            height: 16px;
            border-radius: 50%;
            background: #fff;
            border: 3px solid var(--accent);
            box-shadow: 0 0 10px var(--accent-glow);
            transition: left 0.8s ease;
        }

        .temp-bar-range {
            display: flex;
            justify-content: space-between;
            margin-top: 6px;
            font-size: 10px;
            color: var(--text-muted);
        }

        /* ── Fan Card ───────────────────────────────── */
        .fan-card {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            text-align: center;
        }

        .fan-svg-container {
            width: 160px;
            height: 160px;
            margin-bottom: 16px;
            transition: filter 0.5s ease;
        }

        .fan-svg-container.fan-active {
            filter: drop-shadow(0 0 20px rgba(0, 230, 118, 0.4));
        }

        .fan-blades {
            transform-origin: center;
            transition: color 0.4s ease;
        }

        .fan-blades.spinning {
            animation: spin 1.5s linear infinite;
        }

        .fan-blade {
            fill: var(--fan-off);
            transition: fill 0.5s ease;
        }

        .fan-active .fan-blade {
            fill: var(--fan-on);
        }

        .fan-hub {
            transition: fill 0.5s ease;
        }

        .fan-status {
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 14px;
            font-weight: 600;
            letter-spacing: 1px;
            text-transform: uppercase;
            color: var(--text-muted);
            transition: color 0.4s ease;
        }

        .fan-status.active {
            color: var(--fan-on);
        }

        .fan-status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background: var(--fan-off);
            transition: background 0.4s ease, box-shadow 0.4s ease;
        }

        .fan-status.active .fan-status-dot {
            background: var(--fan-on);
            box-shadow: 0 0 8px var(--fan-on-glow);
        }

        /* ── Controls Card ──────────────────────────── */
        .controls-card {
            padding: 28px 32px;
        }

        .control-row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 16px 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
        }

        .control-row:last-child {
            border-bottom: none;
        }

        .control-label {
            font-size: 13px;
            font-weight: 500;
            color: var(--text-secondary);
            min-width: 90px;
        }

        .control-right {
            display: flex;
            align-items: center;
            gap: 12px;
            flex: 1;
            justify-content: flex-end;
        }

        /* Mode Toggle Buttons */
        .mode-toggle {
            display: flex;
            gap: 4px;
            background: rgba(255, 255, 255, 0.05);
            border-radius: 12px;
            padding: 4px;
        }

        .mode-btn {
            padding: 8px 20px;
            border: none;
            border-radius: 10px;
            font-family: 'Inter', sans-serif;
            font-size: 13px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            background: transparent;
            color: var(--text-muted);
            outline: none;
        }

        .mode-btn:hover {
            color: var(--text-secondary);
            background: rgba(255, 255, 255, 0.05);
        }

        .mode-btn.active {
            background: var(--accent);
            color: #0b0f1e;
            box-shadow: 0 0 16px var(--accent-glow);
        }

        /* Power Button */
        .power-btn-wrapper {
            display: flex;
            align-items: center;
            justify-content: center;
        }

        .power-btn {
            width: 72px;
            height: 72px;
            border-radius: 50%;
            border: 3px solid var(--fan-off);
            background: transparent;
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            transition: all 0.4s ease;
            position: relative;
            overflow: hidden;
            outline: none;
        }

        .power-btn svg {
            width: 28px;
            height: 28px;
            transition: all 0.4s ease;
        }

        .power-btn svg path,
        .power-btn svg line {
            stroke: var(--fan-off);
            transition: stroke 0.4s ease;
        }

        .power-btn.on {
            border-color: var(--fan-on);
            background: rgba(0, 230, 118, 0.12);
            box-shadow: 0 0 24px var(--fan-on-glow), 0 0 48px rgba(0, 230, 118, 0.15);
            animation: pulseGreen 2s ease-in-out infinite;
        }

        .power-btn.on svg path,
        .power-btn.on svg line {
            stroke: var(--fan-on);
        }

        .power-btn.disabled {
            opacity: 0.3;
            cursor: not-allowed;
            animation: none !important;
        }

        .power-btn:not(.disabled):hover {
            transform: scale(1.08);
        }

        .power-btn:not(.disabled):active {
            transform: scale(0.95);
        }

        /* Ripple */
        .power-btn .ripple {
            position: absolute;
            border-radius: 50%;
            background: rgba(255, 255, 255, 0.2);
            transform: scale(0);
            animation: rippleAnim 0.6s ease-out forwards;
            pointer-events: none;
        }

        /* Sliders */
        .slider-group {
            display: flex;
            align-items: center;
            gap: 12px;
            flex: 1;
            max-width: 320px;
        }

        .slider-group input[type="range"] {
            flex: 1;
            -webkit-appearance: none;
            appearance: none;
            height: 6px;
            border-radius: 3px;
            background: rgba(255, 255, 255, 0.1);
            outline: none;
            transition: opacity 0.3s ease;
        }

        .slider-group input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: #fff;
            border: 3px solid var(--accent);
            cursor: pointer;
            box-shadow: 0 0 8px var(--accent-glow);
            transition: transform 0.2s ease;
        }

        .slider-group input[type="range"]::-webkit-slider-thumb:hover {
            transform: scale(1.2);
        }

        .slider-group input[type="range"]::-moz-range-thumb {
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: #fff;
            border: 3px solid var(--accent);
            cursor: pointer;
            box-shadow: 0 0 8px var(--accent-glow);
        }

        .slider-group input[type="range"]::-webkit-slider-runnable-track {
            background: linear-gradient(90deg, var(--accent), rgba(255,255,255,0.1));
            height: 6px;
            border-radius: 3px;
        }

        .slider-value {
            font-size: 14px;
            font-weight: 600;
            color: var(--accent);
            min-width: 52px;
            text-align: right;
            font-variant-numeric: tabular-nums;
        }

        .slider-disabled {
            opacity: 0.3;
            pointer-events: none;
        }

        /* ── Status Bar ─────────────────────────────── */
        .status-bar {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 32px;
            flex-wrap: wrap;
            animation: fadeSlideUp 0.7s ease-out 0.4s both;
        }

        .status-item {
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 12px;
            color: var(--text-muted);
            font-weight: 400;
        }

        .status-item .label {
            font-weight: 500;
            color: var(--text-secondary);
        }

        .status-item .value {
            font-variant-numeric: tabular-nums;
        }

        .health-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 12px;
        }

        .health-item {
            background: rgba(255, 255, 255, 0.025);
            border: 1px solid var(--card-border);
            border-radius: 14px;
            padding: 12px 14px;
            display: flex;
            flex-direction: column;
            gap: 5px;
        }

        .health-item .health-label {
            font-size: 10px;
            text-transform: uppercase;
            letter-spacing: 0.8px;
            color: var(--text-muted);
            font-weight: 700;
        }

        .health-item .health-value {
            font-size: 13px;
            font-weight: 700;
            color: var(--text-primary);
            font-variant-numeric: tabular-nums;
            word-break: break-word;
        }

        .health-ok { color: var(--success) !important; }
        .health-warn { color: #ffb74d !important; }
        .health-bad { color: var(--danger) !important; }
        .health-info { color: var(--accent) !important; }

        /* ── Animations ─────────────────────────────── */
        @keyframes spin {
            to { transform: rotate(360deg); }
        }

        @keyframes pulseGreen {
            0%, 100% { box-shadow: 0 0 8px var(--fan-on-glow); }
            50% { box-shadow: 0 0 16px var(--fan-on-glow), 0 0 32px rgba(0, 230, 118, 0.2); }
        }

        @keyframes fadeSlideUp {
            from {
                opacity: 0;
                transform: translateY(20px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        @keyframes fadeSlideDown {
            from {
                opacity: 0;
                transform: translateY(-12px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        @keyframes rippleAnim {
            to {
                transform: scale(4);
                opacity: 0;
            }
        }

        /* ── Alert Banners ───────────────────────────── */
        .alert-banner {
            display: flex;
            align-items: center;
            gap: 16px;
            padding: 16px 24px;
            border-radius: 16px;
            margin-bottom: 20px;
            font-size: 14px;
            font-weight: 600;
            backdrop-filter: blur(20px);
            -webkit-backdrop-filter: blur(20px);
            box-shadow: var(--card-shadow);
            animation: slideDownFade 0.5s cubic-bezier(0.25, 0.8, 0.25, 1) both;
            border: 1px solid transparent;
            transition: all 0.5s ease;
        }

        .alarm-banner {
            background: rgba(255, 82, 82, 0.12);
            border-color: rgba(255, 82, 82, 0.35);
            color: #ff8a80;
            box-shadow: 0 8px 32px rgba(255, 82, 82, 0.15);
            animation: slideDownFade 0.5s cubic-bezier(0.25, 0.8, 0.25, 1) both, alertPulse 2s infinite ease-in-out;
        }

        .fault-banner {
            background: rgba(255, 152, 0, 0.12);
            border-color: rgba(255, 152, 0, 0.35);
            color: #ffd180;
            box-shadow: 0 8px 32px rgba(255, 152, 0, 0.15);
            animation: slideDownFade 0.5s cubic-bezier(0.25, 0.8, 0.25, 1) both, faultPulse 2s infinite ease-in-out;
        }

        .cooldown-banner {
            background: rgba(0, 212, 255, 0.10);
            border-color: rgba(0, 212, 255, 0.35);
            color: #80e0ff;
            box-shadow: 0 8px 32px rgba(0, 212, 255, 0.15);
            animation: slideDownFade 0.5s cubic-bezier(0.25, 0.8, 0.25, 1) both;
        }

        /* ── Runtime Section ───────────────────────── */
        .runtime-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 14px;
            margin-top: 14px;
        }

        @media (max-width: 720px) {
            .runtime-grid { grid-template-columns: repeat(2, 1fr); }
        }

        .runtime-stat {
            background: rgba(255, 255, 255, 0.025);
            border: 1px solid var(--card-border);
            border-radius: 14px;
            padding: 14px 16px;
            display: flex;
            flex-direction: column;
            gap: 6px;
            transition: border-color 0.3s ease, background 0.3s ease;
        }

        .runtime-stat .stat-label {
            font-size: 11px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            color: var(--text-muted);
            font-weight: 600;
        }

        .runtime-stat .stat-value {
            font-size: 22px;
            font-weight: 700;
            font-variant-numeric: tabular-nums;
            color: var(--text-primary);
            letter-spacing: -0.5px;
        }

        .runtime-stat.cooldown-active {
            background: rgba(0, 212, 255, 0.08);
            border-color: rgba(0, 212, 255, 0.35);
        }

        .runtime-stat.cooldown-active .stat-value {
            color: var(--accent);
        }

        .runtime-progress {
            margin-top: 10px;
            height: 6px;
            background: rgba(255, 255, 255, 0.06);
            border-radius: 999px;
            overflow: hidden;
        }

        .runtime-progress > div {
            height: 100%;
            width: 0%;
            background: linear-gradient(90deg, var(--accent), var(--fan-on));
            border-radius: 999px;
            transition: width 0.6s ease;
        }

        .runtime-progress.danger > div {
            background: linear-gradient(90deg, #ff9800, var(--danger));
        }

        .override-btn {
            margin-top: 14px;
            padding: 10px 18px;
            font-size: 13px;
            font-weight: 600;
            background: rgba(0, 212, 255, 0.10);
            color: var(--accent);
            border: 1px solid rgba(0, 212, 255, 0.35);
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.2s ease;
            font-family: inherit;
        }

        .override-btn:hover:not(:disabled) {
            background: rgba(0, 212, 255, 0.18);
            transform: translateY(-1px);
        }

        .override-btn:disabled {
            opacity: 0.35;
            cursor: not-allowed;
        }

        .hide {
            display: none !important;
        }

        @keyframes slideDownFade {
            from {
                opacity: 0;
                transform: translateY(-20px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        @keyframes alertPulse {
            0%, 100% {
                border-color: rgba(255, 82, 82, 0.35);
                box-shadow: 0 8px 32px rgba(255, 82, 82, 0.15);
            }
            50% {
                border-color: rgba(255, 82, 82, 0.7);
                box-shadow: 0 8px 32px rgba(255, 82, 82, 0.3), 0 0 16px rgba(255, 82, 82, 0.1);
            }
        }

        @keyframes faultPulse {
            0%, 100% {
                border-color: rgba(255, 152, 0, 0.35);
                box-shadow: 0 8px 32px rgba(255, 152, 0, 0.15);
            }
            50% {
                border-color: rgba(255, 152, 0, 0.7);
                box-shadow: 0 8px 32px rgba(255, 152, 0, 0.3), 0 0 16px rgba(255, 152, 0, 0.1);
            }
        }

        /* ── Password Modal & Lock Overlay ───────────── */
        .modal-overlay {
            position: fixed;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: rgba(11, 15, 30, 0.7);
            backdrop-filter: blur(8px);
            -webkit-backdrop-filter: blur(8px);
            display: flex;
            align-items: center;
            justify-content: center;
            z-index: 2000;
            animation: fadeIn 0.3s ease-out both;
        }

        .modal-card {
            width: 90%;
            max-width: 400px;
            background: #131832;
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 20px;
            box-shadow: 0 20px 50px rgba(0, 0, 0, 0.5);
            padding: 28px;
            animation: scaleUp 0.3s cubic-bezier(0.34, 1.56, 0.64, 1) both;
        }

        .modal-header {
            display: flex;
            align-items: center;
            gap: 12px;
            margin-bottom: 12px;
        }

        .modal-header h3 {
            font-size: 18px;
            font-weight: 600;
            color: #fff;
        }

        .modal-card p {
            font-size: 13px;
            color: var(--text-secondary);
            line-height: 1.5;
            margin-bottom: 20px;
        }

        .modal-card input[type="password"] {
            width: 100%;
            background: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 12px;
            padding: 12px 16px;
            font-family: 'Inter', sans-serif;
            color: #fff;
            font-size: 14px;
            outline: none;
            transition: border-color 0.3s ease, box-shadow 0.3s ease;
            margin-bottom: 12px;
        }

        .modal-card input[type="password"]:focus {
            border-color: var(--accent);
            box-shadow: 0 0 8px var(--accent-glow);
        }

        .password-error {
            font-size: 12px;
            font-weight: 600;
            color: var(--danger);
            margin-bottom: 16px;
            animation: shake 0.3s ease;
        }

        .modal-buttons {
            display: flex;
            gap: 12px;
            justify-content: flex-end;
        }

        .modal-buttons button {
            padding: 10px 20px;
            border-radius: 10px;
            font-size: 13px;
            font-weight: 600;
            cursor: pointer;
            border: none;
            transition: all 0.2s ease;
        }

        .modal-btn-submit {
            background: var(--accent);
            color: #0b0f1e;
            box-shadow: 0 0 12px var(--accent-glow);
        }

        .modal-btn-submit:hover {
            transform: scale(1.03);
        }

        .modal-btn-cancel {
            background: rgba(255, 255, 255, 0.05);
            color: var(--text-secondary);
            border: 1px solid rgba(255, 255, 255, 0.08) !important;
        }

        .modal-btn-cancel:hover {
            background: rgba(255, 255, 255, 0.1);
            color: #fff;
        }

        .lock-overlay {
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            z-index: 10;
            cursor: pointer;
            background: transparent;
        }

        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }

        @keyframes scaleUp {
            from { opacity: 0; transform: scale(0.9); }
            to { opacity: 1; transform: scale(1); }
        }

        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            25% { transform: translateX(-5px); }
            75% { transform: translateX(5px); }
        }

        /* ── Responsive ─────────────────────────────── */
        @media (max-width: 768px) {
            .dashboard-grid {
                grid-template-columns: 1fr;
            }

            .container {
                padding: 16px 12px 32px;
            }

            header {
                padding: 14px 18px;
                border-radius: 16px;
            }

            .header-title {
                font-size: 15px;
            }

            .card {
                padding: 24px;
                border-radius: 16px;
            }

            .temp-value {
                font-size: 56px;
            }

            .fan-svg-container {
                width: 130px;
                height: 130px;
            }

            .control-row {
                flex-direction: column;
                align-items: flex-start;
                gap: 10px;
            }

            .control-right {
                width: 100%;
                justify-content: flex-start;
            }

            .slider-group {
                max-width: 100%;
                width: 100%;
            }

            .status-bar {
                gap: 16px;
            }

            .health-grid {
                grid-template-columns: repeat(2, 1fr);
            }

            .power-btn {
                width: 64px;
                height: 64px;
            }
        }

        /* ── Scrollbar ──────────────────────────────── */
        ::-webkit-scrollbar {
            width: 6px;
        }
        ::-webkit-scrollbar-track {
            background: transparent;
        }
        ::-webkit-scrollbar-thumb {
            background: rgba(255,255,255,0.1);
            border-radius: 3px;
        }
    </style>
</head>
<body>
    <div class="container">
        <!-- Header -->
        <header>
            <div class="header-title">
                <span class="icon">🌡️</span>
                Smart Fan Controller
            </div>
            <div class="connection-status">
                <div id="statusDot" class="status-dot"></div>
                <span id="statusText">Disconnected</span>
            </div>
        </header>

        <!-- Warning Banners -->
        <div id="alarmBanner" class="alert-banner alarm-banner hide">
            <span class="icon" style="font-size: 20px;">🔥</span>
            <span class="banner-text">HIGH TEMPERATURE ALARM: Room temperature has exceeded critical limit!</span>
        </div>
        <div id="faultBanner" class="alert-banner fault-banner hide">
            <span class="icon" style="font-size: 20px;">⚠️</span>
            <span class="banner-text">CONTACTOR FAULT: Feedback mismatch! Contactor auxiliary contact state does not match commanded state.</span>
        </div>
        <div id="cooldownBanner" class="alert-banner cooldown-banner hide">
            <span class="icon" style="font-size: 20px;">⚠️</span>
            <span class="banner-text">MOTOR REST CYCLE: Fan ran continuously past its limit. Forced OFF for <span id="cooldownBannerTime">—</span> to protect the motor.</span>
        </div>

        <!-- Main Grid -->
        <div class="dashboard-grid">
            <!-- Temperature Card -->
            <section class="card temp-card" aria-label="Temperature and Humidity">
                <div class="card-label">Temperature</div>
                <div>
                    <span id="tempValue" class="temp-value">--</span><span class="temp-unit">°C</span>
                </div>
                <div class="humidity-row">
                    <span class="icon">💧</span>
                    <span id="humidityValue">--%</span>
                </div>
                <div class="humidity-bar-container">
                    <div class="humidity-bar-bg">
                        <div id="humidityBar" class="humidity-bar-fill"></div>
                    </div>
                </div>
                <div class="temp-bar-container">
                    <div class="temp-bar-label">Temperature Range</div>
                    <div class="temp-bar">
                        <div id="tempBarMarker" class="temp-bar-marker" style="left:0%"></div>
                    </div>
                    <div class="temp-bar-range">
                        <span>10°C</span>
                        <span>45°C</span>
                    </div>
                </div>
            </section>

            <!-- Fan Card -->
            <section class="card fan-card" aria-label="Fan Status">
                <div class="card-label">Fan</div>
                <div id="fanSvgContainer" class="fan-svg-container">
                    <svg viewBox="0 0 200 200" xmlns="http://www.w3.org/2000/svg">
                        <defs>
                            <radialGradient id="hubGradient" cx="50%" cy="50%" r="50%">
                                <stop offset="0%" stop-color="#3e4468"/>
                                <stop offset="100%" stop-color="#121626"/>
                            </radialGradient>
                            <linearGradient id="bladeGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                                <stop offset="0%" stop-color="#5ff9a7"/>
                                <stop offset="40%" stop-color="#00e676"/>
                                <stop offset="100%" stop-color="#00793c"/>
                            </linearGradient>
                            <linearGradient id="bladeOffGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                                <stop offset="0%" stop-color="#a0a5ba"/>
                                <stop offset="40%" stop-color="#555a70"/>
                                <stop offset="100%" stop-color="#2a2e3d"/>
                            </linearGradient>
                        </defs>
                        <g id="fanBlades" class="fan-blades" transform-origin="100 100">
                            <!-- Blade 1 (up) -->
                            <g class="fan-blade-group">
                                <path class="fan-blade" d="M100,90 Q95,50 80,20 Q100,10 120,20 Q105,50 100,90 Z" fill="url(#bladeOffGradient)" opacity="0.95"/>
                                <path d="M100,90 Q97,50 90,20 Q100,10 105,20 Q100,50 100,90 Z" fill="#ffffff" opacity="0.12"/>
                            </g>
                            <!-- Blade 2 (upper-right) -->
                            <g class="fan-blade-group">
                                <path class="fan-blade" d="M108,95 Q140,75 170,65 Q178,85 170,105 Q140,90 108,95 Z" fill="url(#bladeOffGradient)" opacity="0.95"/>
                                <path d="M108,95 Q130,85 155,75 Q160,85 155,95 Q130,90 108,95 Z" fill="#ffffff" opacity="0.12"/>
                            </g>
                            <!-- Blade 3 (lower-right) -->
                            <g class="fan-blade-group">
                                <path class="fan-blade" d="M105,108 Q130,130 145,160 Q125,170 105,160 Q120,130 105,108 Z" fill="url(#bladeOffGradient)" opacity="0.95"/>
                                <path d="M105,108 Q120,120 130,145 Q120,150 115,145 Q115,125 105,108 Z" fill="#ffffff" opacity="0.12"/>
                            </g>
                            <!-- Blade 4 (lower-left) -->
                            <g class="fan-blade-group">
                                <path class="fan-blade" d="M92,108 Q65,130 50,160 Q70,170 92,160 Q77,130 92,108 Z" fill="url(#bladeOffGradient)" opacity="0.95"/>
                                <path d="M92,108 Q77,120 70,145 Q80,150 85,145 Q85,125 92,108 Z" fill="#ffffff" opacity="0.12"/>
                            </g>
                            <!-- Blade 5 (upper-left) -->
                            <g class="fan-blade-group">
                                <path class="fan-blade" d="M92,95 Q60,75 30,65 Q22,85 30,105 Q60,90 92,95 Z" fill="url(#bladeOffGradient)" opacity="0.95"/>
                                <path d="M92,95 Q70,85 45,75 Q40,85 45,95 Q70,90 92,95 Z" fill="#ffffff" opacity="0.12"/>
                            </g>
                        </g>
                        <!-- Center Hub -->
                        <circle class="fan-hub" cx="100" cy="100" r="16" fill="url(#hubGradient)" stroke="rgba(255,255,255,0.15)" stroke-width="1.5"/>
                        <circle cx="100" cy="100" r="6" fill="rgba(255,255,255,0.08)"/>
                    </svg>
                </div>
                <div id="fanStatus" class="fan-status">
                    <div class="fan-status-dot"></div>
                    <span id="fanStatusText">Fan is OFF</span>
                </div>
                <!-- Cycle Protection Timer Badge -->
                <div id="cycleTimerContainer" class="status-item hide" style="margin-top: 14px; font-size: 11px; color: var(--accent); background: rgba(0, 212, 255, 0.08); padding: 5px 12px; border-radius: 8px; border: 1px solid rgba(0, 212, 255, 0.2); align-items: center; gap: 6px;">
                    <span class="label" style="color: var(--accent); font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px;">Cycle Lock:</span>
                    <span id="cycleTimerValue" class="value" style="font-weight: 700; color: #fff;">12s</span>
                </div>
            </section>
        </div>

        <!-- Fan State Card (read-only status driven by physical switch) -->
        <section class="card controls-card card-full" aria-label="Fan State" style="margin-top:24px;">
            <div class="card-label">Fan State</div>

            <!-- Mode -->
            <div class="control-row">
                <div class="control-label">Mode</div>
                <div class="control-right">
                    <div class="mode-toggle" style="opacity: 0.95; pointer-events: none;">
                        <button id="btnAuto" class="mode-btn active" type="button" disabled>AUTO</button>
                        <button id="btnManual" class="mode-btn" type="button" disabled>MANUAL</button>
                    </div>
                </div>
            </div>

            <!-- Physical Switch State -->
            <div class="control-row">
                <div class="control-label">Physical Switch</div>
                <div class="control-right">
                    <span id="physicalSwitchValue" class="slider-value" style="color: #fff; font-size: 13px; font-weight: 700; letter-spacing: 0.5px; background: rgba(255,255,255,0.06); padding: 6px 14px; border-radius: 10px; border: 1px solid rgba(255,255,255,0.1); width: 100px; text-align: center; display: inline-block;">OFF</span>
                </div>
            </div>

            <!-- Power -->
            <div class="control-row">
                <div class="control-label">Power</div>
                <div class="control-right">
                    <div class="power-btn-wrapper">
                        <button id="powerBtn" class="power-btn disabled" type="button" aria-label="Fan Power Indicator" disabled>
                            <svg viewBox="0 0 24 24" fill="none" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
                                <path d="M18.36 6.64a9 9 0 1 1-12.73 0"/>
                                <line x1="12" y1="2" x2="12" y2="12"/>
                            </svg>
                        </button>
                    </div>
                </div>
            </div>

            <div style="margin-top: 8px; padding: 10px 14px; background: rgba(255,255,255,0.03); border-left: 3px solid #5ff9a7; border-radius: 6px; font-size: 12px; color: rgba(255,255,255,0.65); line-height: 1.5;">
                Read-only. Mode and power are driven by the physical AUTO/OFF/ON selector switch on the device.
            </div>
        </section>

        <!-- Runtime & Rest Cycle -->
        <section class="card card-full" aria-label="Fan Runtime" style="margin-top:24px;">
            <div class="card-label">Fan Runtime</div>

            <div class="runtime-grid">
                <div id="runtimeCurrent" class="runtime-stat">
                    <span class="stat-label">Current Run</span>
                    <span id="currentRunValue" class="stat-value">—</span>
                </div>
                <div class="runtime-stat">
                    <span class="stat-label">Today Runtime</span>
                    <span id="totalRunValue" class="stat-value">—</span>
                </div>
                <div class="runtime-stat">
                    <span class="stat-label">On/Off Cycles</span>
                    <span id="cycleCountValue" class="stat-value">—</span>
                </div>
                <div id="runtimeCooldown" class="runtime-stat">
                    <span class="stat-label">Rest Cooldown</span>
                    <span id="cooldownValue" class="stat-value">—</span>
                </div>
            </div>

            <div id="runtimeProgress" class="runtime-progress"><div id="runtimeProgressBar"></div></div>

            <button id="overrideBtn" class="override-btn hide" onclick="requestEndCooldown()" disabled>End Cooldown</button>
        </section>

        <!-- Status Bar -->
        <section class="card status-bar card-full" aria-label="System Status" style="margin-top:24px; padding:18px 28px;">
            <div class="status-item">
                <span class="label">IP</span>
                <span id="ipValue" class="value">—</span>
            </div>
            <div class="status-item">
                <span class="label">Signal</span>
                <span id="rssiValue" class="value">— dBm</span>
            </div>
            <div class="status-item">
                <span class="label">Uptime</span>
                <span id="uptimeValue" class="value">—</span>
            </div>
        </section>

        <!-- Health Overview -->
        <section class="card card-full" aria-label="Controller Health" style="margin-top:24px;">
            <div class="card-label">Controller Health</div>
            <div class="health-grid">
                <div class="health-item">
                    <span class="health-label">Sensor</span>
                    <span id="sensorHealthValue" class="health-value">—</span>
                </div>
                <div class="health-item">
                    <span class="health-label">Last Sensor Update</span>
                    <span id="sensorAgeValue" class="health-value">—</span>
                </div>
                <div class="health-item">
                    <span class="health-label">WiFi Quality</span>
                    <span id="wifiQualityValue" class="health-value">—</span>
                </div>
                <div class="health-item">
                    <span class="health-label">Feedback Contact</span>
                    <span id="feedbackStatusValue" class="health-value">—</span>
                </div>
                <div class="health-item">
                    <span class="health-label">Controller State</span>
                    <span id="controllerStateValue" class="health-value">—</span>
                </div>
                <div class="health-item">
                    <span class="health-label">Active Lock</span>
                    <span id="activeLockValue" class="health-value">—</span>
                </div>
                <div class="health-item">
                    <span class="health-label">Last Event</span>
                    <span id="lastEventValue" class="health-value">—</span>
                </div>
                <div class="health-item">
                    <span class="health-label">Firmware</span>
                    <span id="firmwareValue" class="health-value">—</span>
                </div>
            </div>
        </section>
        <!-- Settings Card -->
        <section class="card controls-card card-full" aria-label="Settings" style="margin-top:24px;">
            <div class="card-label">Settings</div>

            <!-- Threshold -->
            <div id="thresholdRow" class="control-row">
                <div class="control-label" style="cursor: pointer; display: flex; align-items: center; gap: 6px;" onclick="openPasswordModal()">
                    Threshold <span id="thresholdLockIcon" style="font-size: 11px;">🔒</span>
                </div>
                <div class="control-right">
                    <div class="slider-group" id="thresholdSliderGroup" style="position: relative;">
                        <div id="thresholdOverlay" class="lock-overlay" onclick="openPasswordModal()"></div>
                        <input type="range" id="thresholdSlider" min="18" max="40" step="0.5" value="28" oninput="previewThreshold(this.value)" onchange="updateThreshold(this.value)">
                        <span id="thresholdValue" class="slider-value">28.0°C</span>
                    </div>
                </div>
            </div>

            <!-- Hysteresis -->
            <div id="hysteresisRow" class="control-row">
                <div class="control-label" style="cursor: pointer; display: flex; align-items: center; gap: 6px;" onclick="openPasswordModal()">
                    Hysteresis <span id="hysteresisLockIcon" style="font-size: 11px;">🔒</span>
                </div>
                <div class="control-right">
                    <div class="slider-group" id="hysteresisSliderGroup" style="position: relative;">
                        <div id="hysteresisOverlay" class="lock-overlay" onclick="openPasswordModal()"></div>
                        <input type="range" id="hysteresisSlider" min="0.5" max="5" step="0.5" value="2" oninput="previewHysteresis(this.value)" onchange="updateHysteresis(this.value)">
                        <span id="hysteresisValue" class="slider-value">2.0°C</span>
                    </div>
                </div>
            </div>
        </section>

        <!-- Rest Cycle Settings -->
        <section class="card controls-card card-full" aria-label="Rest Cycle Settings" style="margin-top:24px;">
            <div class="card-label">Rest Cycle</div>

            <div id="maxRunRow" class="control-row">
                <div class="control-label">
                    Max Continuous Run <span id="maxRunLockIcon" style="font-size: 11px;">🔒</span>
                </div>
                <div class="control-right">
                    <div class="slider-group" id="maxRunSliderGroup" style="position: relative;">
                        <div id="maxRunOverlay" class="lock-overlay" onclick="openPasswordModal()"></div>
                        <input type="range" id="maxRunSlider" min="60" max="1440" step="30" value="720" oninput="previewMaxRun(this.value)" onchange="updateMaxRun(this.value)">
                        <span id="maxRunValue" class="slider-value">12h 0m</span>
                    </div>
                </div>
            </div>

            <div id="cooldownRow" class="control-row">
                <div class="control-label">
                    Cooldown Duration <span id="cooldownLockIcon" style="font-size: 11px;">🔒</span>
                </div>
                <div class="control-right">
                    <div class="slider-group" id="cooldownSliderGroup" style="position: relative;">
                        <div id="cooldownOverlay" class="lock-overlay" onclick="openPasswordModal()"></div>
                        <input type="range" id="cooldownSlider" min="5" max="120" step="5" value="30" oninput="previewCooldownDuration(this.value)" onchange="updateCooldownDuration(this.value)">
                        <span id="cooldownDurValue" class="slider-value">30m</span>
                    </div>
                </div>
            </div>
        </section>

    </div>

    <!-- Password Lock Modal -->
    <div id="passwordModal" class="modal-overlay hide" onclick="closePasswordModal(event)">
        <div class="modal-card" onclick="event.stopPropagation()">
            <div class="modal-header">
                <span class="icon" style="font-size: 18px;">🔒</span>
                <h3>Settings Locked</h3>
            </div>
            <p>Enter the administrator password to unlock temperature and hysteresis adjustments:</p>
            <input type="password" id="adminPasswordInput" placeholder="Password" onkeydown="handlePasswordKey(event)">
            <div id="passwordError" class="password-error hide">❌ Invalid password. Please try again.</div>
            <div class="modal-buttons">
                <button class="modal-btn-submit" onclick="submitPassword()">Unlock</button>
                <button class="modal-btn-cancel" onclick="closePasswordModal()">Cancel</button>
            </div>
        </div>
    </div>

    <script>
        /* ── State ──────────────────────────────────── */
        let ws = null;
        let isAutoMode = true;
        let isFanOn = false;
        let isAuthenticated = false;

        /* ── Fan Physics Simulation (Inertia + Motion Blur) ── */
        let fanAngle = 0;
        let fanSpeed = 0;       // Current speed in degrees per frame
        const maxSpeed = 16;    // Maximum speed (degrees per frame)
        const accel = 0.08;     // Acceleration rate
        const decel = 0.04;     // Deceleration/coasting friction

        function animateFan() {
            const targetSpeed = isFanOn ? maxSpeed : 0;

            if (isFanOn) {
                if (fanSpeed < targetSpeed) {
                    fanSpeed += accel;
                    if (fanSpeed > targetSpeed) fanSpeed = targetSpeed;
                }
            } else {
                if (fanSpeed > 0) {
                    fanSpeed -= decel;
                    if (fanSpeed < 0) fanSpeed = 0;
                }
            }

            const fanBlades = document.getElementById('fanBlades');
            if (fanSpeed > 0) {
                fanAngle = (fanAngle + fanSpeed) % 360;
                fanBlades.style.transform = `rotate(${fanAngle}deg)`;
                
                // Motion blur filter proportional to speed
                const blurVal = Math.min(2.5, fanSpeed * 0.15);
                fanBlades.style.filter = blurVal > 0.15 ? `blur(${blurVal}px)` : 'none';
            } else {
                fanBlades.style.filter = 'none';
            }

            requestAnimationFrame(animateFan);
        }

        // Initialize the physics loop
        animateFan();

        /* ── WebSocket Connection ───────────────────── */
        function connect() {
            const host = location.hostname || 'localhost';
            ws = new WebSocket('ws://' + host + ':81');

            ws.onopen = function() {
                document.getElementById('statusDot').classList.add('connected');
                document.getElementById('statusText').textContent = 'Connected';
                document.getElementById('ipValue').textContent = host;
            };

            ws.onmessage = function(evt) {
                try {
                    const data = JSON.parse(evt.data);
                    if (data.action === 'auth_res') {
                        setAuthSuccess(data.success);
                    } else if (data.action === 'auth_req') {
                        isAuthenticated = false;
                        openPasswordModal();
                    } else if (data.action === 'error') {
                        console.warn('Controller error:', data.message || 'Unknown error');
                    } else {
                        updateDashboard(data);
                    }
                } catch(e) {
                    console.error('Parse error:', e);
                }
            };

            ws.onclose = function() {
                document.getElementById('statusDot').classList.remove('connected');
                document.getElementById('statusText').textContent = 'Disconnected';
                
                // Relock settings on connection loss
                isAuthenticated = false;
                document.getElementById('thresholdLockIcon').textContent = '🔒';
                document.getElementById('hysteresisLockIcon').textContent = '🔒';
                document.getElementById('maxRunLockIcon').textContent = '🔒';
                document.getElementById('cooldownLockIcon').textContent = '🔒';
                document.getElementById('thresholdOverlay').classList.remove('hide');
                document.getElementById('hysteresisOverlay').classList.remove('hide');
                document.getElementById('maxRunOverlay').classList.remove('hide');
                document.getElementById('cooldownOverlay').classList.remove('hide');
                
                setTimeout(connect, 3000);
            };

            ws.onerror = function() {
                ws.close();
            };
        }

        /* ── Dashboard Update ───────────────────────── */
        function updateDashboard(data) {
            // Temperature
            const tempEl = document.getElementById('tempValue');
            const temp = parseFloat(data.temp).toFixed(1);
            animateValue(tempEl, temp);
            applyTempGradient(tempEl, data.temp);

            // Temperature bar marker
            const pct = Math.max(0, Math.min(100, ((data.temp - 10) / 35) * 100));
            document.getElementById('tempBarMarker').style.left = pct + '%';

            // Humidity
            const hum = parseFloat(data.humidity).toFixed(0);
            document.getElementById('humidityValue').textContent = hum + '%';
            document.getElementById('humidityBar').style.width = hum + '%';

            // Fan state
            isFanOn = data.fan;
            const fanContainer = document.getElementById('fanSvgContainer');
            const fanStatus = document.getElementById('fanStatus');
            const fanStatusText = document.getElementById('fanStatusText');
            const powerBtn = document.getElementById('powerBtn');

            // Toggle SVG gradient definitions and active states
            const blades = document.querySelectorAll('.fan-blade');
            if (isFanOn) {
                fanContainer.classList.add('fan-active');
                fanStatus.classList.add('active');
                fanStatusText.textContent = 'Fan is ON';
                powerBtn.classList.add('on');
                blades.forEach(b => b.setAttribute('fill', 'url(#bladeGradient)'));
            } else {
                fanContainer.classList.remove('fan-active');
                fanStatus.classList.remove('active');
                fanStatusText.textContent = 'Fan is OFF';
                powerBtn.classList.remove('on');
                blades.forEach(b => b.setAttribute('fill', 'url(#bladeOffGradient)'));
            }

            // Physical Switch
            const physicalSwitch = data.switch || "AUTO";
            const switchEl = document.getElementById('physicalSwitchValue');
            switchEl.textContent = physicalSwitch;
            if (physicalSwitch === 'AUTO') {
                switchEl.style.borderColor = 'var(--accent)';
                switchEl.style.color = 'var(--accent)';
            } else if (physicalSwitch === 'ON') {
                switchEl.style.borderColor = 'var(--success)';
                switchEl.style.color = 'var(--success)';
            } else {
                switchEl.style.borderColor = 'var(--danger)';
                switchEl.style.color = 'var(--danger)';
            }

            // Mode UI
            isAutoMode = data.mode === 'auto';
            updateModeUI(isAutoMode);

            // Sliders
            document.getElementById('thresholdSlider').value = data.threshold;
            document.getElementById('thresholdValue').textContent = parseFloat(data.threshold).toFixed(1) + '°C';
            document.getElementById('hysteresisSlider').value = data.hysteresis;
            document.getElementById('hysteresisValue').textContent = parseFloat(data.hysteresis).toFixed(1) + '°C';

            // Alarms & Fault Banners
            const alarmBanner = document.getElementById('alarmBanner');
            if (data.alarm) {
                alarmBanner.classList.remove('hide');
            } else {
                alarmBanner.classList.add('hide');
            }

            const faultBanner = document.getElementById('faultBanner');
            if (data.fault) {
                faultBanner.classList.remove('hide');
            } else {
                faultBanner.classList.add('hide');
            }

            // Cycle Protection Lock indicator
            const cycleContainer = document.getElementById('cycleTimerContainer');
            const cycleValue = document.getElementById('cycleTimerValue');
            if (data.lock_type && data.lock_type !== 'none' && data.lock_time > 0) {
                cycleContainer.classList.remove('hide');
                cycleValue.textContent = data.lock_time + 's (' + (data.lock_type === 'run' ? 'HOLD ON' : 'HOLD OFF') + ')';
            } else {
                cycleContainer.classList.add('hide');
            }

            // --- Runtime Section ---
            const curRun = (typeof data.current_run_sec === 'number') ? data.current_run_sec : 0;
            const totRun = (typeof data.total_run_sec === 'number') ? data.total_run_sec : 0;
            const cycles = (typeof data.cycle_count === 'number') ? data.cycle_count : 0;
            document.getElementById('currentRunValue').textContent = formatDuration(curRun);
            document.getElementById('totalRunValue').textContent = formatDuration(totRun);
            document.getElementById('cycleCountValue').textContent = cycles;

            const maxRunSec = (typeof data.max_cont_run_sec === 'number') ? data.max_cont_run_sec : 43200;
            const cooldownSec = (typeof data.cooldown_sec === 'number') ? data.cooldown_sec : 1800;

            // Progress bar shows current run / max continuous run
            const progressEl = document.getElementById('runtimeProgress');
            const progressBar = document.getElementById('runtimeProgressBar');
            const runPct = Math.min(100, (curRun / Math.max(1, maxRunSec)) * 100);
            progressBar.style.width = runPct.toFixed(1) + '%';
            if (runPct >= 85) {
                progressEl.classList.add('danger');
            } else {
                progressEl.classList.remove('danger');
            }

            // Cooldown card + banner
            const cooldownActive = !!data.cooldown_active;
            const cooldownRemaining = (typeof data.cooldown_remaining_sec === 'number') ? data.cooldown_remaining_sec : 0;
            const cooldownCard = document.getElementById('runtimeCooldown');
            const cooldownVal = document.getElementById('cooldownValue');
            const overrideBtn = document.getElementById('overrideBtn');
            const cooldownBanner = document.getElementById('cooldownBanner');
            const cooldownBannerTime = document.getElementById('cooldownBannerTime');

            if (cooldownActive) {
                cooldownCard.classList.add('cooldown-active');
                cooldownVal.textContent = formatDuration(cooldownRemaining) + ' left';
                overrideBtn.classList.remove('hide');
                overrideBtn.disabled = !isAuthenticated;
                cooldownBanner.classList.remove('hide');
                cooldownBannerTime.textContent = formatDuration(cooldownRemaining);
            } else {
                cooldownCard.classList.remove('cooldown-active');
                cooldownVal.textContent = 'Inactive';
                overrideBtn.classList.add('hide');
                overrideBtn.disabled = true;
                cooldownBanner.classList.add('hide');
            }

            // Rest Cycle sliders (avoid clobbering while user is dragging)
            const maxRunSlider = document.getElementById('maxRunSlider');
            if (document.activeElement !== maxRunSlider) {
                const maxRunMin = Math.round(maxRunSec / 60);
                maxRunSlider.value = maxRunMin;
                document.getElementById('maxRunValue').textContent = formatMinutes(maxRunMin);
            }
            const cooldownSlider = document.getElementById('cooldownSlider');
            if (document.activeElement !== cooldownSlider) {
                const cdMin = Math.round(cooldownSec / 60);
                cooldownSlider.value = cdMin;
                document.getElementById('cooldownDurValue').textContent = formatMinutes(cdMin);
            }

            // Status bar
            document.getElementById('rssiValue').textContent = data.rssi + ' dBm';
            document.getElementById('uptimeValue').textContent = formatUptime(data.uptime);

            // Health overview
            updateHealthValue('sensorHealthValue', formatSensorStatus(data.sensor_status, data.sensor_fail_safe_active), healthClassForSensor(data.sensor_status));
            document.getElementById('sensorAgeValue').textContent =
                typeof data.sensor_last_update_sec === 'number' ? formatDuration(data.sensor_last_update_sec) + ' ago' : '—';
            updateHealthValue('wifiQualityValue', titleCase(data.wifi_quality || 'unknown'), healthClassForWifi(data.wifi_quality));
            updateHealthValue('feedbackStatusValue', data.feedback_closed ? 'Closed' : 'Open', data.fault ? 'health-bad' : 'health-ok');
            updateHealthValue('controllerStateValue', formatState(data.controller_state), data.fault ? 'health-bad' : 'health-info');
            updateHealthValue('activeLockValue', formatState(data.active_lock_reason || 'none'), data.active_lock_reason && data.active_lock_reason !== 'none' ? 'health-warn' : 'health-ok');
            document.getElementById('lastEventValue').textContent = formatState(data.last_event || '—');
            document.getElementById('firmwareValue').textContent = data.firmware_version || '—';
        }

        /* ── Mode UI ────────────────────────────────── */
        function updateModeUI(isAuto) {
            const btnAuto = document.getElementById('btnAuto');
            const btnManual = document.getElementById('btnManual');
            const powerBtn = document.getElementById('powerBtn');
            const thresholdGroup = document.getElementById('thresholdSliderGroup');
            const hysteresisGroup = document.getElementById('hysteresisSliderGroup');

            // Web overrides are disabled. Physical switch owns configuration modes.
            btnAuto.classList.remove('active');
            btnManual.classList.remove('active');
            
            if (isAuto) {
                btnAuto.classList.add('active');
            } else {
                btnManual.classList.add('active');
            }
            
            // Power button is always disabled under physical control
            powerBtn.classList.add('disabled');
            
            // Adjust slider controls accessibility based on AUTO/MANUAL modes
            if (isAuto) {
                thresholdGroup.classList.remove('slider-disabled');
                hysteresisGroup.classList.remove('slider-disabled');
            } else {
                thresholdGroup.classList.add('slider-disabled');
                hysteresisGroup.classList.add('slider-disabled');
            }
        }

        /* ── Settings Security ──────────────────────── */
        function openPasswordModal() {
            if (isAuthenticated) return;
            document.getElementById('passwordModal').classList.remove('hide');
            document.getElementById('adminPasswordInput').value = '';
            document.getElementById('passwordError').classList.add('hide');
            setTimeout(() => document.getElementById('adminPasswordInput').focus(), 100);
        }

        function closePasswordModal(e) {
            if (e && e.target !== document.getElementById('passwordModal')) return;
            document.getElementById('passwordModal').classList.add('hide');
        }

        function handlePasswordKey(e) {
            if (e.key === 'Enter') {
                submitPassword();
            } else if (e.key === 'Escape') {
                closePasswordModal();
            }
        }

        function submitPassword() {
            const pwd = document.getElementById('adminPasswordInput').value;
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ action: 'auth', value: pwd }));
            }
        }

        function setAuthSuccess(success) {
            if (success) {
                isAuthenticated = true;
                document.getElementById('passwordModal').classList.add('hide');
                document.getElementById('thresholdLockIcon').textContent = '🔓';
                document.getElementById('hysteresisLockIcon').textContent = '🔓';
                document.getElementById('thresholdOverlay').classList.add('hide');
                document.getElementById('hysteresisOverlay').classList.add('hide');
                document.getElementById('maxRunLockIcon').textContent = '🔓';
                document.getElementById('cooldownLockIcon').textContent = '🔓';
                document.getElementById('maxRunOverlay').classList.add('hide');
                document.getElementById('cooldownOverlay').classList.add('hide');
                const ob = document.getElementById('overrideBtn');
                if (!ob.classList.contains('hide')) ob.disabled = false;
            } else {
                const err = document.getElementById('passwordError');
                err.classList.remove('hide');
                const card = document.querySelector('.modal-card');
                card.style.animation = 'none';
                setTimeout(() => card.style.animation = 'shake 0.3s ease', 10);
            }
        }

        /* ── Commands ───────────────────────────────── */
        function setMode(isAuto) {
            console.log('Mode toggle ignored. Use physical AUTO/OFF/ON selector switch.');
        }

        function toggleFan() {
            console.log('Manual toggle ignored. Use physical AUTO/OFF/ON selector switch.');
        }

        function previewThreshold(val) {
            document.getElementById('thresholdValue').textContent = parseFloat(val).toFixed(1) + '°C';
        }

        function previewHysteresis(val) {
            document.getElementById('hysteresisValue').textContent = parseFloat(val).toFixed(1) + '°C';
        }

        function previewMaxRun(val) {
            document.getElementById('maxRunValue').textContent = formatMinutes(parseInt(val, 10));
        }

        function previewCooldownDuration(val) {
            document.getElementById('cooldownDurValue').textContent = formatMinutes(parseInt(val, 10));
        }

        function updateThreshold(val) {
            if (!isAuthenticated) return;
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ action: 'threshold', value: parseFloat(val) }));
            }
            previewThreshold(val);
        }

        function updateHysteresis(val) {
            if (!isAuthenticated) return;
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ action: 'hysteresis', value: parseFloat(val) }));
            }
            previewHysteresis(val);
        }

        function updateMaxRun(val) {
            // Slider is in minutes; firmware accepts minutes as value
            const mins = parseInt(val, 10);
            previewMaxRun(mins);
            if (!isAuthenticated) return;
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ action: 'max_run', value: mins }));
            }
        }

        function updateCooldownDuration(val) {
            const mins = parseInt(val, 10);
            previewCooldownDuration(mins);
            if (!isAuthenticated) return;
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ action: 'cooldown', value: mins }));
            }
        }

        function requestEndCooldown() {
            if (!isAuthenticated) {
                openPasswordModal();
                return;
            }
            if (!confirm('End the motor rest cycle early?\n\nThis bypasses the safety cooldown and may shorten the motor\u2019s lifespan if used frequently.')) {
                return;
            }
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ action: 'end_cooldown' }));
            }
        }

        /* ── Power Button Handler ───────────────────── */
        function handlePowerClick(e) {
            console.log('Power controls overridden by physical AUTO/OFF/ON switch.');
        }

        /* ── Helpers ────────────────────────────────── */
        function formatUptime(seconds) {
            if (!seconds && seconds !== 0) return '—';
            const h = Math.floor(seconds / 3600);
            const m = Math.floor((seconds % 3600) / 60);
            if (h > 0) return h + 'h ' + m + 'm';
            return m + 'm';
        }

        function formatDuration(seconds) {
            if (seconds == null || isNaN(seconds)) return '—';
            seconds = Math.max(0, Math.floor(seconds));
            const h = Math.floor(seconds / 3600);
            const m = Math.floor((seconds % 3600) / 60);
            const s = seconds % 60;
            if (h > 0) return h + 'h ' + String(m).padStart(2, '0') + 'm';
            if (m > 0) return m + 'm ' + String(s).padStart(2, '0') + 's';
            return s + 's';
        }

        function formatMinutes(mins) {
            mins = Math.max(0, Math.floor(mins));
            const h = Math.floor(mins / 60);
            const m = mins % 60;
            if (h > 0 && m > 0) return h + 'h ' + m + 'm';
            if (h > 0) return h + 'h';
            return m + 'm';
        }

        function getTemperatureColor(temp) {
            if (temp < 20) return '#2196f3';
            if (temp < 25) return '#4caf50';
            if (temp < 30) return '#ff9800';
            return '#f44336';
        }

        function applyTempGradient(el, temp) {
            const c1 = getTemperatureColor(temp - 2);
            const c2 = getTemperatureColor(temp);
            el.style.background = 'linear-gradient(135deg, ' + c1 + ', ' + c2 + ')';
            el.style.webkitBackgroundClip = 'text';
            el.style.webkitTextFillColor = 'transparent';
            el.style.backgroundClip = 'text';
        }

        function animateValue(el, newVal) {
            el.textContent = newVal;
        }

        function titleCase(value) {
            return String(value || '—')
                .replace(/_/g, ' ')
                .replace(/\b\w/g, ch => ch.toUpperCase());
        }

        function formatState(value) {
            if (!value || value === '—') return '—';
            return titleCase(value);
        }

        function formatSensorStatus(status, failSafeActive) {
            if (failSafeActive) return 'Fail-Safe Active';
            return titleCase(status || 'unknown');
        }

        function healthClassForSensor(status) {
            if (status === 'ok') return 'health-ok';
            if (status === 'stale' || status === 'unknown') return 'health-warn';
            return 'health-bad';
        }

        function healthClassForWifi(quality) {
            if (quality === 'excellent' || quality === 'good') return 'health-ok';
            if (quality === 'fair' || quality === 'provisioning') return 'health-warn';
            if (quality === 'offline' || quality === 'weak') return 'health-bad';
            return 'health-info';
        }

        function updateHealthValue(id, text, className) {
            const el = document.getElementById(id);
            if (!el) return;
            el.classList.remove('health-ok', 'health-warn', 'health-bad', 'health-info');
            if (className) el.classList.add(className);
            el.textContent = text || '—';
        }

        /* ── Init ───────────────────────────────────── */
        connect();

    </script>
</body>
</html>
)rawliteral";

#endif
