
import { useEffect, useState } from 'react';
import { ListGroup, Button } from "react-bootstrap";

import { useToaster } from './toaster';
import { Spinner } from "./Spinner";
import { AnimationListEntry, PatternListEntry } from './Pattern';
import { PatternLinks } from './PatternLinks';
import { useNavigate } from 'react-router-dom';
import { activateAnimation, activatePattern, getAnimationList, getPatternList } from './api';

export function PatternsPage(): JSX.Element {

    const [loading, setLoading] = useState(true);
    const [patterns, setPatterns] = useState<PatternListEntry[]>([]);
    const [animations, setAnimations] = useState<AnimationListEntry[]>([]);
    const navigate = useNavigate();

    const toaster = useToaster();

    useEffect(() => {
        let cancelled = false;

        async function getPatterns() {

            try {
                const response: PatternListEntry[] = await getPatternList();
                if (!cancelled) {
                    setPatterns(response);

                    const animList: AnimationListEntry[] = await getAnimationList();
                    if(!cancelled) {
                        // Do something with animations if needed
                        setAnimations(animList);
                    }
                }
                
            }
            catch(e: any) {
                toaster.open("Failed to read", (<><p>"The list of custom patterns could not be read. Check the device is running."</p><p>{e.toString()}</p></>));
            }
            finally{
                setLoading(false);
            }
        };

        getPatterns();

        return () => {
            cancelled = true;
        };
    }, [toaster]);


    return (
        <div>
            <h1 className="mt-4">
                Patterns
            </h1>
            <Spinner loading={loading}></Spinner>
            <div style={{ position: 'relative', marginRight: '60px'  }}>
                <ListGroup style={{ position: 'relative', zIndex: 0 }}>
                    {patterns.map((pattern) => (
                        <ListGroup.Item 
                            key={pattern.id} 
                            style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}
                            onClick={(e) => {
                                // Only navigate if not clicking the Activate button
                                if (!(e.target instanceof HTMLButtonElement)) {
                                    navigate(`/patterns/edit/${pattern.id}`);
                                }
                            }}
                        >
                            <span>{pattern.name}</span>
                            <Button
                                onClick={async (e) => {
                                    e.stopPropagation();
                                    try {
                                        setLoading(true);
                                        const success = await activatePattern(pattern.id);
                                        if (!success) {
                                            toaster.open("Failed to activate", "The pattern could not be activated. The request may be invalid.");
                                        }
                                    } catch (error) {
                                        toaster.open("Failed to activate", "The pattern could not be activated. Check the device is running.");
                                    } finally {
                                        setLoading(false);
                                    }
                                }}
                                size="sm"
                                variant="primary"
                            >
                                Activate
                            </Button>
                        </ListGroup.Item>
                    ))}
                </ListGroup>
                <PatternLinks 
                    patterns={patterns}
                    itemHeight={40} // Standard Bootstrap ListGroup.Item height
                    rightOffset={0} // Distance from right edge to start arrows
                />
            </div>
            <Button className="mt-4" onClick={() => navigate("/patterns/add")}>Add Pattern</Button>

            <h1 className="mt-4">
                Animations
            </h1>

            <div style={{ position: 'relative', marginRight: '60px'  }}>
                <ListGroup>
                    {animations.map((animation) => (
                        <ListGroup.Item 
                            key={animation.id} 
                            style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}
                        >
                            <span>{animation.name}</span>
                            <Button
                                onClick={async (e) => {
                                    e.stopPropagation();
                                    try {
                                        setLoading(true);
                                        const success = await activateAnimation(animation.id);
                                        if (!success) {
                                            toaster.open("Failed to activate", "The animation could not be activated. The request may be invalid.");
                                        }
                                    } catch (error) {
                                        toaster.open("Failed to activate", "The animation could not be activated. Check the device is running.");
                                    } finally {
                                        setLoading(false);
                                    }
                                }}
                                size="sm"
                                variant="primary"
                            >
                                Activate
                            </Button>
                        </ListGroup.Item>
                    ))}
                </ListGroup>
            </div>
        </div>
    );
}