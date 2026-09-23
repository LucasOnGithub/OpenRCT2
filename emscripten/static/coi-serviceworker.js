/*****************************************************************************
 * Copyright (c) 2014-2026 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

// OpenRCT2's Emscripten build uses pthreads, which require a cross-origin
// isolated page. GitHub Pages cannot set COOP/COEP response headers itself,
// so this file doubles as a tiny service worker that adds them to same-origin
// responses. On first visit the page registers the worker and reloads once.

if (typeof window === "undefined")
{
    self.addEventListener("install", () => self.skipWaiting());

    self.addEventListener("activate", (event) =>
    {
        event.waitUntil(self.clients.claim());
    });

    self.addEventListener("fetch", (event) =>
    {
        // Chromium can issue this combination for requests that service workers
        // are not allowed to handle.
        if (event.request.cache === "only-if-cached" && event.request.mode !== "same-origin")
        {
            return;
        }

        event.respondWith((async () =>
        {
            const response = await fetch(event.request);

            // Opaque cross-origin responses cannot be reconstructed.
            if (response.status === 0)
            {
                return response;
            }

            const headers = new Headers(response.headers);
            headers.set("Cross-Origin-Opener-Policy", "same-origin");
            headers.set("Cross-Origin-Embedder-Policy", "require-corp");

            return new Response(response.body, {
                status: response.status,
                statusText: response.statusText,
                headers
            });
        })());
    });
}
else
{
    const reloadKey = "openrct2-coi-reload";

    if (window.crossOriginIsolated)
    {
        sessionStorage.removeItem(reloadKey);
    }
    else if ("serviceWorker" in navigator)
    {
        const workerUrl = document.currentScript.src;

        navigator.serviceWorker.register(workerUrl)
            .then(() => navigator.serviceWorker.ready)
            .then(() =>
            {
                // One reload lets the now-active service worker control the
                // top-level document and attach COOP/COEP headers to it.
                if (sessionStorage.getItem(reloadKey) !== "1")
                {
                    sessionStorage.setItem(reloadKey, "1");
                    window.location.reload();
                }
            })
            .catch((error) => console.error("Unable to enable cross-origin isolation:", error));
    }
}
