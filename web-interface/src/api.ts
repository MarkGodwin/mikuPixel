import { PatternListEntry, PatternConfig, AnimationListEntry } from './Pattern';

export async function getPatternList(): Promise<PatternListEntry[]> {
    const response = await fetch("/api/patterns/list.json");
    if (!response.ok) {
        throw new Error("Failed to load pattern list");
    }
    return await response.json();
}

export async function getPattern(id: number): Promise<PatternConfig> {
    const response = await fetch(`/api/patterns/get.json?id=${id}`);
    if (!response.ok) {
        throw new Error("Failed to load pattern");
    }
    return await response.json();
}

export async function beginEditPattern(id: number): Promise<boolean> {
    const response = await fetch(`/api/patterns/beginEdit.json?id=${id}`);
    if (!response.ok) {
        throw new Error("Failed to edit pattern");
    }
    return await response.json();
}

export async function endEditPattern(id: number, name: string, nextFrame: number | null, frameTime: number, transitionTime: number): Promise<boolean> {
    const params = new URLSearchParams({
        id: id.toString(),
        name,
        nextFrame: nextFrame !== null ? nextFrame.toString() : "",
        frameTime: frameTime.toString(),
        transition: transitionTime.toString()
    });
    const response = await fetch(`/api/patterns/endEdit.json?${params.toString()}`);
    if (!response.ok) {
        throw new Error("Failed to save pattern");
    }
    return await response.json();
}

export async function updatePixel(index: number, red: number, green: number, blue: number): Promise<void> {
    await fetch(`/api/patterns/setLed.json?index=${index}&r=${red.toString()}&g=${green.toString()}&b=${blue.toString()}`);
}

export async function activatePattern(id: number): Promise<boolean> {
    const response = await fetch(`/api/activatePattern.json?id=${id}`);
    if (!response.ok) {
        throw new Error("Failed to activate pattern");
    }
    return await response.json();
}

export async function getAnimationList(): Promise<AnimationListEntry[]> {
    const response = await fetch("/api/animations/list.json");
    if (!response.ok) {
        throw new Error("Failed to load animation list");
    }
    return await response.json();
}

export async function activateAnimation(id: number): Promise<boolean> {
    const response = await fetch(`/api/activateAnimation.json?id=${id}`);
    if (!response.ok) {
        throw new Error("Failed to activate animation");
    }
    return await response.json();
}

export async function setRgb(red: number, green: number, blue: number): Promise<boolean> {
    const response = await fetch(`/api/setRgb.json?r=${red.toString()}&g=${green.toString()}&b=${blue.toString()}`);
    if (!response.ok) {
        throw new Error("Failed to set RGB");
    }
    return await response.json();
}

export async function setBrightness(value: number): Promise<boolean> {
    const response = await fetch(`/api/setBrightness.json?value=${value.toString()}`);
    if (!response.ok) {
        throw new Error("Failed to set brightness");
    }
    return await response.json();
}