(pyodide) => {
  const width = window.screen.width * window.devicePixelRatio;
  const height = window.screen.height * window.devicePixelRatio;
  const canvas = document.createElement('canvas');

  canvas.id = 'canvas';
  canvas.width = width;
  canvas.height = height;

  canvas.style.position = 'fixed';
  canvas.style.left = '0';
  canvas.style.top = '0';
  canvas.style.width = '100vw';
  canvas.style.height = '100vh';

  const options = {
    powerPreference: 'high-performance',
    premultipliedAlpha: false,
    antialias: false,
    alpha: false,
    depth: false,
    stencil: false,
  };

  const gl = canvas.getContext('webgl2', options);
  document.body.appendChild(canvas);

  const setupRender = (update) => {
    const render = () => {
      update();
      requestAnimationFrame(render);
    };

    requestAnimationFrame(render);
  };

  return { gl, width, height, setupRender };
}
