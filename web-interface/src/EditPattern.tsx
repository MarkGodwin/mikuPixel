
import { useCallback, useEffect, useState, useRef } from 'react';
import { Button, Form } from 'react-bootstrap';
import { useParams } from 'react-router-dom';
import { useToaster } from './toaster';
import { PatternConfig, PatternListEntry, PixelPositions, RgbPixel } from './Pattern';
import PatternEditor from './PatternEditor';
import LoadingOverlay from './LoadingOverlay';
import { getPattern, getPatternList, beginEditPattern, endEditPattern, updatePixel } from './api';

class PixelUpdateQueue {
    private queue: Array<{index: number, color: RgbPixel}> = [];
    private processingPromise : Promise<void> | null = null;

    async waitForProcessing() {
        if (this.processingPromise) {
            await this.processingPromise;
        }
    }

    add(index: number, color: RgbPixel) {
        const existingIndex = this.queue.findIndex(item => item.index === index);
        if (existingIndex >= 0) {
            this.queue[existingIndex].color = color;
        } else {
            this.queue.push({index, color});
        }

        if (!this.processingPromise) {
            this.processingPromise = this.processQueue();
        }
    }

    private async processQueue() {
        while (this.queue.length > 0) {
            const update = this.queue.shift()!;
            try {
                await updatePixel(update.index, update.color.r, update.color.g, update.color.b);
            } catch (error) {
                console.error('Failed to update pixel:', error);
            }
        }
        this.processingPromise = null;
    }
}

const EditPatternPage = () => {
    const [pattern, setPattern] = useState<PatternConfig|null>(null);
    const [patternList, setPatternList] = useState<PatternListEntry[]>([]);
    const [isEditing, setIsEditing] = useState(false);
    const [loading, setLoading] = useState(true);
    //const [frameTime, setFrameTime] = useState(1000);
    //const [nextFrame, setNextFrame] = useState<number|null>(null);

    const {id} = useParams<{id: string}>();
    const idNumber = parseInt(id || "-1", 10);
    const toaster = useToaster();
    const updateQueue = useRef(new PixelUpdateQueue());

    useEffect(() => {
        async function doLoad() {
            try {
                setLoading(true);
                const [pattern, patternList] = await Promise.all([
                    getPattern(idNumber),
                    getPatternList()
                ]);
                setPattern(pattern);
                setPatternList(patternList);
                //setFrameTime(pattern.frameTime);
                //setNextFrame(pattern.nextFrame);
            } catch (error) {
                toaster.open("Failed to load pattern", "The pattern could not be loaded. Check the device is running.");
            } finally {
                setLoading(false);
            }
        }
        doLoad();
    }, [idNumber, toaster]);

    const beginEdit = useCallback(async () => {
        if(isEditing) {
            toaster.open("Already editing", "You are already editing this pattern.");
            return;
        }

        setLoading(true);
        try {
            const success = await beginEditPattern(idNumber);
            if (!success) {
                toaster.open("Failed to begin edit", "The pattern could not be locked for editing. Check the device is running.");
                return;
            }
            setIsEditing(true);
        } finally {
            setLoading(false);
        }
    }, [idNumber, toaster, isEditing]);

    const endEdit = useCallback(async () => {
        if(!isEditing || !pattern) {
            toaster.open("Not editing", "You are not currently editing this pattern.");
            return;
        }

        setLoading(true);
        try {
            // Ensure all pixel updates are processed
            await updateQueue.current.waitForProcessing();
            
            // Save configuration changes
            const success = await endEditPattern(idNumber, pattern.name, pattern.nextFrame, pattern.frameTime, pattern.transitionTime);
            if (!success) {
                toaster.open("Failed to end edit", "The pattern could not be unlocked for editing. Check the device is running.");
                return;
            }
            setIsEditing(false);
        } finally {
            setLoading(false);
        }
    }, [idNumber, toaster, isEditing, pattern]);

    const onPixelChange = useCallback((index: number, color: RgbPixel) => {
        if(!pattern) return;
        const newPixels = [...pattern.pixels];
        newPixels[index] = color;
        setPattern({...pattern, pixels: newPixels});
        
        updateQueue.current.add(index, color);
    }, [pattern]);

    return (
        <LoadingOverlay loading={loading}>
            <h1 className="mt-4">Pattern Editor</h1>
            
            <Form className="mb-4">
                <Form.Group className="mb-3">
                    <Form.Label>Name</Form.Label>
                    <Form.Control 
                        type="text" 
                        value={pattern?.name || ''} 
                        readOnly={!isEditing}
                        onChange={(e) => pattern && setPattern({ ...pattern, name: e.target.value })}
                    />
                </Form.Group>

                <Form.Group className="mb-3">
                    <Form.Label>Frame Duration (ms)</Form.Label>
                    <Form.Control 
                        type="number" 
                        value={pattern?.frameTime}
                        onChange={(e) => pattern && setPattern( { ...pattern, frameTime: parseInt(e.target.value, 10)})}
                        disabled={!isEditing}
                        min={0}
                        step={100}
                    />
                </Form.Group>

                <Form.Group className="mb-3">
                    <Form.Label>Transition Time (ms)</Form.Label>
                    <Form.Control 
                        type="number" 
                        value={pattern?.transitionTime}
                        onChange={(e) => pattern && setPattern( { ...pattern, transitionTime: parseInt(e.target.value, 10)})}
                        disabled={!isEditing}
                        min={0}
                        step={100}
                    />
                </Form.Group>

                <Form.Group className="mb-3">
                    <Form.Label>Next Frame</Form.Label>
                    <Form.Select
                        value={pattern?.nextFrame === null ? '' : pattern?.nextFrame}
                        onChange={(e) => pattern && setPattern( { ...pattern, nextFrame: e.target.value === '' ? null : parseInt(e.target.value, 10) })}
                        disabled={!isEditing}
                    >
                        <option value="">None</option>
                        {patternList.map(p => (
                            <option key={p.id} value={p.id}>
                                {p.name}
                            </option>
                        ))}
                    </Form.Select>
                </Form.Group>
            </Form>

            {pattern && 
                <PatternEditor 
                    isEditing={isEditing} 
                    pixels={pattern.pixels} 
                    onPixelChange={onPixelChange} 
                    pixelPositions={PixelPositions} 
                    backgroundImageUrl="/MikuLedOutline.png" 
                />
            }

            {isEditing ? (
                <Button className="mt-4" onClick={endEdit}>
                    Save Pattern
                </Button>
            ) : (
                <Button className="mt-4" onClick={beginEdit}>
                    Edit Pattern
                </Button>
            )}
        </LoadingOverlay>
    );
}

export default EditPatternPage;