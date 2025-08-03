import { Spinner } from 'react-bootstrap';

const overlayStyle: React.CSSProperties = {
  position: 'absolute',
  top: 0,
  bottom: 0,
  left: 0,
  right: 0,
  backgroundColor: 'rgba(255, 255, 255, 0.6)',
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  zIndex: 1000,
};

interface LoadingOverlayProps {
  loading: boolean;
  children: React.ReactNode;
  dimOpacity?: number; // e.g., 0.3
}

const LoadingOverlay: React.FC<LoadingOverlayProps> = ({ loading, children, dimOpacity = 0.3 }) => (
  <div style={{ position: 'relative' }}>
    {loading && <div style={overlayStyle}><Spinner animation="border" variant="primary" /></div>}
    <div style={{ opacity: loading ? dimOpacity : 1, transition: 'opacity 0.3s' }}>
      {children}
    </div>
  </div>
);

export default LoadingOverlay;