require 'test_helper'

class AudioCategoriesControllerTest < ActionController::TestCase
  setup do
    @audio_category = audio_categories(:one)
  end

  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:audio_categories)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create audio_category" do
    assert_difference('AudioCategory.count') do
      post :create, audio_category: {  }
    end

    assert_redirected_to audio_category_path(assigns(:audio_category))
  end

  test "should show audio_category" do
    get :show, id: @audio_category
    assert_response :success
  end

  test "should get edit" do
    get :edit, id: @audio_category
    assert_response :success
  end

  test "should update audio_category" do
    put :update, id: @audio_category, audio_category: {  }
    assert_redirected_to audio_category_path(assigns(:audio_category))
  end

  test "should destroy audio_category" do
    assert_difference('AudioCategory.count', -1) do
      delete :destroy, id: @audio_category
    end

    assert_redirected_to audio_categories_path
  end
end
