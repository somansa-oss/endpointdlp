Contributing Guide
==================
소만사 오픈소스 프로젝트에는 누구나 기여자가 될 수 있습니다.

버그 리포팅 방법
-----------
GitHub의 [Issue](https://github.com/SomansaOpenSource/endpointdlp/issues) 를 이용하여 프로젝트에서 발견된 버그를 리포팅 할 수 있습니다. 
리포팅 시에는 다음의 내용이 포함되어야 합니다.
>* 버그 요약
>* 재현 방법(가능한 구체적으로)
>* 예상 결과 및 실제 현상
>* 기타 참고 사항

Pull Request 체크리스트
-------------------------
Pull Request를 보내기 전에, 다음의 체크리스트가 충족되었는지 확인 해야합니다.
>* 프로젝트의 기여하기 가이드라인 읽기
>* [Code of Conduct](https://github.com/SomansaOpenSource/endpointdlp/blob/master/CODE_OF_CONDUCT.md) 읽기
>* [기여자 라이선스 동의서(CLA, Contributor License Agreement)](https://github.com/SomansaOpenSource/endpointdlp/blob/master/CONTRIBUTOR_LICENSE_AGREEMENT.MD)를 읽고 동의하기
>* 소만사 [코딩 가이드라인](https://github.com/SomansaOpenSource/endpointdlp/blob/master/CONTRIBUTUNG.md#%EC%86%8C%EB%A7%8C%EC%82%AC-%EC%BD%94%EB%94%A9-%EA%B0%80%EC%9D%B4%EB%93%9C%EB%9D%BC%EC%9D%B8c-%EC%96%B8%EC%96%B4---naming-rules) 읽기

Pull Request 시작하기
--------
>1. 'Fork'를 통해 원본 Repository를 나의 Repository로 가져옵니다.
>2. 'Clone'을 통해 내 Repository를 로컬 저장소로 가져옵니다.
>3. 로컬에서 작업한 결과를 'Commit'하고, 'Push'를 통해 나의 Repository에 업로드 합니다.
>4. 'Pull Request'를 통해 나의 작업 내용을 원본 Repository에 반영 요청합니다.

소만사 코딩 가이드라인(C 언어) - Naming Rules
------------------
  >#### 1. 일반
  >* 함수 이름, 변수 이름, 파일 이름은 약어를 사용하지 않고 서술형으로 짓는다.
  >#### 2. 파일 이름
  >* 파일 이름은 모두 소문자로 구성하고 대시('-')를 포함할 수 있다.
  >* 이미 존재하는 파일 이름은 사용하지 않는다.
  >* 파일 이름은 되도록 상세하게 짓는다.
  >* 인라인 함수는 헤더 파일에 정의한다.
  >#### 3. 디렉터리 이름
  >* 디렉터리 이름은 모두 소문자로 구성하고 대시('-')를 포함할 수 있다.
  >#### 4. 타입 이름
  >* 타입 이름은 대문자로 시작하며, 언더바('_')없이 단어마다 첫글자를 대문자로 작성한다.
  >#### 5. 변수 이름 
  >* 변수 이름은 모두 소문자로 작성하며 단어 사이에 언더바를 사용한다.
  >* 전역변수는 'g_'와 같이 구분가능한 접두어를 사용한다.
  >#### 6. 상수 이름
  >* 상수 이름은 k로 시작하고, 대소문자가 섞인 형태를 사용한다.
  >#### 7. 함수 이름
  >* 함수 이름은 대문자로 시작하고 각 단어의 첫글자를 대문자로 쓰며, 언더바는 사용하지 않는다.
