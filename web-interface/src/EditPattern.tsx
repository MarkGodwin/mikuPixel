
import { useCallback, useEffect, useState, useRef } from 'react';
import { Button, Form } from 'react-bootstrap';
import { useParams } from 'react-router-dom';
import { useToaster } from './toaster';
import { PatternConfig, PixelPositions } from './Pattern';
import PatternEditor from './PatternEditor';
import LoadingOverlay from './LoadingOverlay';

class PixelUpdateQueue {
    private queue: Array<{index: number, color: number}> = [];
    private processingPromise : Promise<void> | null = null;

    async waitForProcessing() {
        if (this.processingPromise) {
            await this.processingPromise;
        }
    }

    add(index: number, color: number) {
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
                await fetch(`/api/patterns/setLed.json?index=${update.index}&color=${update.color.toString()}`);
            } catch (error) {
                console.error('Failed to update pixel:', error);
            }
        }
        
        this.processingPromise = null;
    }
}

const EditPatternPage = () => {

    const [pattern, setPattern] = useState<PatternConfig|null>(null);
    const [isEditing, setIsEditing] = useState(false);
    const [loading, setLoading] = useState(true);

    const {id} = useParams<{id: string}>();

    const idNumber = parseInt(id || "-1", 10);
    const toaster = useToaster();

    useEffect(() => {
        async function doLoad() {
            const response = await fetch(`/api/patterns/get.json?id=${idNumber}`);
            if (!response.ok) {
                toaster.open("Failed to load pattern", "The pattern could not be loaded. Check the device is running.");
                return;
            }
            const patternData: PatternConfig = await response.json();
            setPattern(patternData);
            setLoading(false);
        }
        doLoad();

    }, [idNumber, toaster]);

    const beginEdit = useCallback(async () => {

        if(isEditing) {
            toaster.open("Already editing", "You are already editing this pattern.");
            return;
        }

        setLoading(true);
        const response = await fetch(`/api/patterns/beginEdit.json?id=${idNumber}`);
        const isOk = response.ok || ! await response.json();
        setLoading(false);
        if(!isOk) {
            toaster.open("Failed to begin edit", "The pattern could not be locked for editing. Check the device is running.");
            return;
        }

        setIsEditing(true);
    }, [idNumber, toaster, isEditing]);

    const endEdit = useCallback(async () => {
        if(!isEditing) {
            toaster.open("Not editing", "You are not currently editing this pattern.");
            return;
        }

        setLoading(true);

        await updateQueue.current.waitForProcessing();

        const response = await fetch(`/api/patterns/endEdit.json?id=${idNumber}`);
        const isOk = response.ok || ! await response.json();
        setLoading(false);
        if(!isOk) {
            toaster.open("Failed to end edit", "The pattern could not be unlocked for editing. Check the device is running.");
            return;
        }
        setIsEditing(false);
    }, [idNumber, toaster, isEditing]);

    const updateQueue = useRef(new PixelUpdateQueue());

    const onPixelChange = useCallback((index: number, color: number) => {
        if(!pattern) return;
        const newPixels = [...pattern.pixels];
        newPixels[index] = color;
        setPattern({...pattern, pixels: newPixels});

        updateQueue.current.add(index, color);
    }, [pattern]);

    return (
        <LoadingOverlay loading={loading}>
            <h1 className="mt-4">
                Edit Pattern
            </h1>
            { pattern && <PatternEditor isEditing={isEditing} pixels={pattern.pixels} onPixelChange={onPixelChange} pixelPositions={PixelPositions} backgroundImageUrl="/MikuLedOutline.png" /> }
            { isEditing ?
                <Button className="mt-4" onClick={() => endEdit()}>
                    Save Pattern
                </Button>
            :
                <Button className="mt-4" onClick={() => beginEdit()}>
                    Edit Pattern
                </Button>             
            }
        </LoadingOverlay>
    );
}

export default EditPatternPage;