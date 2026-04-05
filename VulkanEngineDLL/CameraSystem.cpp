#include "CameraSystem.h"

CameraSystem& cameraSystem = CameraSystem::Get();

void CameraSystem::CreatePixelPerfectOrthographicCamera(const ivec2& renderResolution, const vec2& worldPosition)
{
    Camera cam{};
    cam.CameraType = kPixelPerfectOrthographicCam;
    cam.Width = static_cast<float>(renderResolution.x);
    cam.Height = static_cast<float>(renderResolution.y);
    cam.AspectRatio = cam.Width / cam.Height;
    cam.Zoom = 1.0f;
    cam.Position = vec3(worldPosition.x, worldPosition.y, 0.0f);
    cam.ViewScreenSize = vec2(renderResolution);
    cam.ProjectionMatrix = glm::ortho(0.0f, cam.Width, cam.Height, 0.0f, -1000.0f, 1000.0f);
    cam.ViewMatrix = glm::translate(glm::mat4(1.0f), -cam.Position);

    CameraList.emplace_back(std::move(cam));
}

void CameraSystem::CreatePerspectiveCamera(const ivec2& renderResolution, const vec3& position)
{
    Camera cam{};
    cam.CameraType = kPerspectiveCam;
    cam.Width = static_cast<float>(renderResolution.x);
    cam.Height = static_cast<float>(renderResolution.y);
    cam.AspectRatio = cam.Width / cam.Height;
    cam.Zoom = 45.0f;         
    cam.Position = position;
    cam.Yaw = -90.0f;
    cam.Pitch = 0.0f;
    cam.Front = vec3(0.0f, 0.0f, -1.0f);
    cam.Up = vec3(0.0f, 1.0f, 0.0f);
    cam.WorldUp = vec3(0.0f, 1.0f, 0.0f);
    cam.ProjectionMatrix = glm::perspective(glm::radians(cam.Zoom), cam.AspectRatio, 0.1f, 10000.0f);
    cam.ProjectionMatrix[1][1] *= -1.0f;
    cam.ViewMatrix = glm::lookAt(cam.Position, cam.Position + cam.Front, cam.Up);

    CameraList.emplace_back(std::move(cam));
}

void CameraSystem::CreateCamera(CameraTypeEnum cameraType, const ivec2& renderResolution, const vec2& position)
{
    if (cameraType == kPixelPerfectOrthographicCam)
    {
        CreatePixelPerfectOrthographicCamera(renderResolution, position);
    }
    else if (cameraType == kPerspectiveCam)
    {
        CreatePerspectiveCamera(renderResolution, vec3(position.x, position.y, 0.0f));
    }
}

void CameraSystem::Update()
{
    Camera& cam = CameraList[ActiveCameraIndex];

    switch (cam.CameraType)
    {
        case kPixelPerfectOrthographicCam:
        {
            if (cam.Zoom != 1.0f)
            {
                vec3 scaledPosition = cam.Position * (1.0f - cam.Zoom);
                cam.ViewMatrix = glm::scale(glm::mat4(1.0f), vec3(cam.Zoom)) *
                    glm::translate(glm::mat4(1.0f), -cam.Position - scaledPosition);
            }
            else
            {
                cam.ViewMatrix = glm::translate(glm::mat4(1.0f), -cam.Position);
            }
            break;
        }

        case kPerspectiveCam:
        {
            vec3 front{};
            front.x = cos(glm::radians(cam.Yaw)) * cos(glm::radians(cam.Pitch));
            front.y = sin(glm::radians(cam.Pitch));
            front.z = sin(glm::radians(cam.Yaw)) * cos(glm::radians(cam.Pitch));
            cam.Front = glm::normalize(front);

            cam.Right = glm::normalize(glm::cross(cam.Front, cam.WorldUp));
            cam.Up = glm::normalize(glm::cross(cam.Right, cam.Front));

            cam.ViewMatrix = glm::lookAt(cam.Position, cam.Position + cam.Front, cam.Up);
            break;
        }
    }
}
